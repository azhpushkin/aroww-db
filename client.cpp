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

#include "proto_dist/commands.pb.h"

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 


void start_messaging(int sockdf) {
    int res;
    char key[128];
    char value[128];
    char input[128];

	std::string command_str;
	DBCommand command;

    while(1) {
		command.clear_key();
		command.clear_value();
		command.clear_type();

        std::cout << "cli> " << std::flush;
		fgets(input, 128, stdin);

        for (char* p = input; *p; ++p)
            *p = tolower(*p);

        if ((res = sscanf(input, "get %s", key)) == 1) {
			command.set_type(DBCommandType::GET);
			command.set_key(std::string(key));
        } else if ((res = sscanf(input, "drop %s", key)) == 1) {
            command.set_type(DBCommandType::DROP);
			command.set_key(std::string(key));
        } else if ((res = sscanf(input, "set %s %s", key, value)) == 2) {
            command.set_type(DBCommandType::SET);
			command.set_key(std::string(key));
			command.set_value(std::string(value));
        } else if ((res = sscanf(input, "q")) == 1) {
			std::cout << "Quitting!.." << std::endl;
            break;
        } else {
			sscanf(input, "%s", input);  // truncate
			std::cout << "Command " << input << " is unknown" << std::endl;	
            continue;
        }

		command.SerializeToString(&command_str);
		send(sockdf, command_str.c_str(), command_str.length(), 0);
        
		memset(input, 0, 128);
        recv(sockdf, input, 128, 0);
        DBCommandResult result;
		result.ParseFromString(std::string(input));
		std::string type;
        if (result.type() == 1) type = "OK";
        if (result.type() == 2) type = "KEY_MISSING";
        if (result.type() == 3) type = "ERROR";
        std::cout << fmt::format("{}(value={},err={})", type, result.value(), result.error_msg()) << std::endl;
    }
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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
    
    start_messaging(sockfd);
    

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);

	return 0;
}
