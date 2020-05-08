#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "aroww.hpp"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// Do not include to header file as not part of the interface
void open_socket(ArowwDB* db);
ArowwResult* get_result(ArowwDB* db);


ArowwDB* aroww_init(char* host, char* port) {
	ArowwDB* db = (ArowwDB*) malloc(sizeof(ArowwDB));
	db->host = strdup(host);
	db->port = strdup(port);
	open_socket(db);
	return db;
}

void aroww_close(ArowwDB* db) {
	close(db->socket_fd);
	free(db->host);
	free(db->port);
	free(db);
}


ArowwResult* aroww_get(ArowwDB* db, char* key) {
	char buffer[512];
	buffer[0] = 'G';
	buffer[1] = strlen(key);
	strcpy(buffer+2, key);
	send(db->socket_fd, buffer, strlen(buffer), 0);
	
    return get_result(db);
}


ArowwResult* aroww_set(ArowwDB* db, char* key, char* value) {
	char buffer[512];
	buffer[0] = 'S';
	buffer[1] = strlen(key);
	strcpy(buffer+2, key);
	buffer[strlen(buffer)] = strlen(value);
	strcpy(buffer+strlen(buffer), value);
	
	send(db->socket_fd, buffer, strlen(buffer), 0);
	return get_result(db);
}

ArowwResult* aroww_drop(ArowwDB* db, char* key) {
    char buffer[512];
	buffer[0] = 'D';
	buffer[1] = strlen(key);
	strcpy(buffer+2, key);
	
	send(db->socket_fd, buffer, strlen(buffer), 0);
    return get_result(db);
}

void aroww_free_result(ArowwResult* res) {
	free(res->value);
	free(res->error_msg);
	free(res);
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void open_socket(ArowwDB* db) {
    int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(db->host, db->port, &hints, &servinfo)) != 0) {
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
    db->socket_fd = sockfd;
}

ArowwResult* get_result(ArowwDB* db) {
    int numbytes;
    char buf[MAXDATASIZE];
    
    if ((numbytes = recv(db->socket_fd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }


    ArowwResult* c_res = (ArowwResult*) malloc(sizeof(ArowwResult));
	c_res->is_ok = (buf[0] == 'O');
	c_res->error_msg = NULL;
	c_res->value = NULL;
	if (buf[0] == 'O') {  // OK
		char kl = buf[2];
		if (kl != 0) {
			c_res->value = strcpy((char*)malloc(sizeof(char) * (kl+1)), buf+3);
		}
	} else {
		char kl = buf[2];
		if (kl != 0) {
			c_res->error_msg = strcpy((char*)malloc(sizeof(char) * (kl+1)), buf+3);
		}
	}
	
    return c_res;
}
