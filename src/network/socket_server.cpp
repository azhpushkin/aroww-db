#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#include "socket_server.hpp"
#include "engine/engine.hpp"

#include "commands.pb.h"


#define BACKLOG 10


struct ThreadParams {
    SimpleSocketServer* server;
    int* socket;
};

SimpleSocketServer::SimpleSocketServer(int port_, AbstractEngine& engine_)
: port(port_),
  engine(engine_)
{}


void SimpleSocketServer::start_listening()
{
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;
    pthread_t thread;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(port); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), 8); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    spdlog::info("Server started, listening on {}", port);

    while (1) { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        spdlog::info("New connection from {}, binding to {}", inet_ntoa(their_addr.sin_addr), new_fd);
        ThreadParams *params = new ThreadParams {this, &new_fd};
        pthread_create(&thread, 0, SimpleSocketServer::start_connection_thread, (void*)params);
        pthread_detach(thread);
    }
}

void* SimpleSocketServer::start_connection_thread(void* params) {
    char buffer[512];
    std::string output;
    int result;
    int socket = *((ThreadParams*)params)->socket;
    SimpleSocketServer* server = ((ThreadParams*)params)->server;

    spdlog::info(">> {}: thread started", socket);
    while (1) {
        memset(buffer, 0, 500);
        result = recv(socket, buffer, 512, 0);
        if (result == 0) {
            spdlog::info(">> {}: connection closed", socket);
            break;
        }
        if (result == -1) {
            perror("Error receiving from connection!");
            break;
        }

        DBCommand command;
        
        command.ParseFromString(std::string(buffer));
        std::string type;
        if (command.type() == 1) type = "GET";
        if (command.type() == 2) type = "SET";
        if (command.type() == 3) type = "DROP";
        spdlog::info(">> {}: received command: {}(key={},value={})", socket, type, command.key(), command.value());

        OpResult opres;
        DBCommandResult result{};
        if (command.type() == 1) {  // GET
            opres = server->engine.get(command.key());
            if (opres.success) {
                result.set_type(ResultType::OK);
                result.set_value(opres.value.value());
            } else {
                result.set_type(ResultType::KEY_MISSING);
                result.set_error_msg(opres.error_msg.value());
            }
        }
        if (command.type() == 2) {
            opres = server->engine.set(command.key(), command.value());
            result.set_type(ResultType::OK);
        }
        if (command.type() == 3) {
            opres = server->engine.drop(command.key());
            result.set_type(ResultType::OK);
        }
        
        output.clear();
        result.SerializeToString(&output);
        spdlog::info(">> {}: Sending back: {}(key={},value={})", socket, result.type(), command.key(), command.value());

        send(socket, output.c_str(), output.length(), 0);
    }

    close(socket);
    pthread_exit(0);
}