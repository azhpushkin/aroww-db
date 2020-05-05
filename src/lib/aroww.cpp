#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <iostream>
#include <fmt/format.h>

#include "commands.pb.h"
#include "aroww.hpp"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

ArowwConnection::ArowwConnection(std::string host_, std::string port_) : host(host_), port(port_) {}

int ArowwConnection::open_conn() {
    int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
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
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("Successfully connected to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure
    fd_socket = sockfd;
	this->get("asd");
    return 0;
}


int ArowwConnection::close_conn() {
    close(fd_socket);
	return 0;
}

ArowwResult ArowwConnection::send_command(DBCommand& command) {
    int numbytes;
    char buf[MAXDATASIZE];
    std::string command_str;
    command.SerializeToString(&command_str);
    send(fd_socket, command_str.c_str(), command_str.length(), 0);
    
    if ((numbytes = recv(fd_socket, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    DBCommandResult result;
    result.ParseFromString(std::string(buf));

    ArowwResult ares{};
    ares.success = (result.type() == 1);
    if (result.has_value()) ares.value = result.value();
    if (result.has_error_msg()) ares.error_msg = result.error_msg();

    return ares;
}
    
ArowwResult ArowwConnection::get(std::string key) {
    DBCommand command;
    command.set_type(DBCommandType::GET);
    command.set_key(key);
    return send_command(command);
}

ArowwResult ArowwConnection::set(std::string key, std::string value) {
    DBCommand command;
    command.set_type(DBCommandType::SET);
    command.set_key(key);
    command.set_value(value);
    return send_command(command);
}

ArowwResult ArowwConnection::drop(std::string key) {
    DBCommand command;
    command.set_type(DBCommandType::DROP);
    command.set_key(key);
    return send_command(command);
}



Connection* open_connection(char* host, char* port) {
	ArowwConnection* cpp_conn = new ArowwConnection{std::string(host), std::string(port)};
	cpp_conn->open_conn();
	Connection* c_conn = new Connection{(void*)cpp_conn};

	return c_conn;
}
void close_connection(Connection* c_conn) {
	ArowwConnection* cpp_conn = (ArowwConnection*)c_conn->cpp_conn;
	cpp_conn->close_conn();
	delete cpp_conn;
	delete c_conn;
}

ConRes* connection_get(Connection* c_conn, char* key) {
	ArowwConnection* cpp_conn = (ArowwConnection*)c_conn->cpp_conn;
	auto res = cpp_conn->get(std::string(key));
	return new ConRes{
		res.success ? 1 : 0,
		res.value.has_value() ? res.value.value().c_str() : "" ,
		res.error_msg.has_value() ? res.error_msg.value().c_str() : "" ,
	};
}
ConRes* connection_set(Connection* c_conn, char* key, char* value) {
	ArowwConnection* cpp_conn = (ArowwConnection*)c_conn->cpp_conn;
	auto res = cpp_conn->set(std::string(key), std::string(value));
	return new ConRes{
		res.success ? 1 : 0,
		res.value.has_value() ? res.value.value().c_str() : "" ,
		res.error_msg.has_value() ? res.error_msg.value().c_str() : "" ,
	};
}
ConRes* connection_drop(Connection* c_conn, char* key) {
	ArowwConnection* cpp_conn = (ArowwConnection*)c_conn->cpp_conn;
	auto res = cpp_conn->drop(std::string(key));
	return new ConRes{
		res.success ? 1 : 0,
		res.value.has_value() ? res.value.value().c_str() : "" ,
		res.error_msg.has_value() ? res.error_msg.value().c_str() : "" ,
	};
}