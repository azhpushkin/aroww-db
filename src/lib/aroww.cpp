#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <memory>

#include "aroww.hpp"
#include "common/messages.hpp"
#include "common/string_or_tomb.hpp"


std::unique_ptr<Message> ArowwDB::get(std::string key) {
	MessageGetRequest msg;
	msg.key = key;

	auto packed = msg.pack_message();
	send(sockfd, packed.c_str(), packed.size(), 0);
	
    return get_result();
}


std::unique_ptr<Message> ArowwDB::set(std::string key, std::string value) {
	MessageSetRequest msg;
	msg.key = key;
	msg.value = value;

	auto packed = msg.pack_message();
	send(sockfd, packed.c_str(), packed.size(), 0);
	
    return get_result();
}

std::unique_ptr<Message> ArowwDB::drop(std::string key) {
	MessageSetRequest msg;
	msg.key = key;
	msg.value = tomb::create();

	auto packed = msg.pack_message();
	send(sockfd, packed.c_str(), packed.size(), 0);
	
    return get_result();
}


ArowwDB::ArowwDB(std::string h, std::string p): host(h), port(p) {
	open_socket();
}

ArowwDB::~ArowwDB() {
	close(sockfd);
}



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void ArowwDB::open_socket() {
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
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
	printf("Successfully connected to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure
}


std::unique_ptr<Message> ArowwDB::get_result() {
    int numbytes;
    char buf[1024];
    
    if ((numbytes = recv(sockfd, buf, 1024-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
	return Message::unpack_message(std::string(buf, numbytes));
}

