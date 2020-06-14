#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <memory>
#include <optional>
#include <variant>

#include "fmt/format.h"

#include "aroww.hpp"
#include "common/messages.hpp"
#include "common/string_or_tomb.hpp"


#define WRONG_TYPE_MSG(EXP, GOT) fmt::format("Wrong message received: expected {:c}, got {:c}!", EXP, GOT)

// Private functions
int open_socket(Aroww::ArowwDB* conn);
std::unique_ptr<Message> get_result(Aroww::ArowwDB* conn);

template<typename T>
std::unique_ptr<T> check_and_convert(std::unique_ptr<Message> msg) {
	if (msg->get_flag() == ERROR_RESP) {
		auto error_msg = std::unique_ptr<MessageErrorResponse>((MessageErrorResponse*)msg.release());
		
		throw Aroww::ArowwException(error_msg->error_msg);
	}
	if (T::get_flag_static() != msg->get_flag()) {
		throw Aroww::ArowwException(WRONG_TYPE_MSG(T::get_flag_static(), msg->get_flag()));
	}

	return std::unique_ptr<T>((T*)msg.release());
}


namespace Aroww {
	ArowwException::ArowwException(std::string m): message(m) {}
	const char * ArowwException::what () const noexcept {
		return message.c_str();
	}

	ArowwDB::ArowwDB(std::string h, std::string p): host(h), port(p), sockfd(0) {
		sockfd = open_socket(this);
	}

	ArowwDB::~ArowwDB() {
		close(sockfd);
	}


	std::optional<std::string> ArowwDB::get(std::string key) {
		MessageGetRequest msg;
		msg.key = key;

		auto packed = msg.pack_message();
		send(sockfd, packed.c_str(), packed.size(), 0);

		auto response_msg = get_result(this);
		auto converted_msg = check_and_convert<MessageGetResponse>(std::move(response_msg));
		
		if (std::holds_alternative<std::string>(converted_msg->value)) {
			return std::get<std::string>(converted_msg->value);
		} else {
			return std::nullopt;
		}
	}


	void ArowwDB::set(std::string key, std::string value) {
		MessageSetRequest msg;
		msg.key = key;
		msg.value = value;

		auto packed = msg.pack_message();
		send(sockfd, packed.c_str(), packed.size(), 0);
		
		auto response_msg = get_result(this);
		check_and_convert<MessageSetResponse>(std::move(response_msg));
	}

	void ArowwDB::drop(std::string key) {
		MessageSetRequest msg;
		msg.key = key;
		msg.value = tomb::create();

		auto packed = msg.pack_message();
		send(sockfd, packed.c_str(), packed.size(), 0);
		
		auto response_msg = get_result(this);
		check_and_convert<MessageSetResponse>(std::move(response_msg));
	}

}  // namespace end


// Private functions

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int open_socket(Aroww::ArowwDB* conn) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(conn->host.c_str(), conn->port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	freeaddrinfo(servinfo); // all done with this structure
	return sockfd;
}


std::unique_ptr<Message> get_result(Aroww::ArowwDB* conn) {
    int numbytes;
    char buf[1024];
    
    if ((numbytes = recv(conn->sockfd, buf, 1024-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	return Message::unpack_message(std::string(buf, numbytes));
}

