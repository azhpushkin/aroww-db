#include <cerrno>
#include <sys/socket.h>
#include <string>

#include <thread>

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#include "socket_server.hpp"
#include "engine/interface.hpp"


RunningConnection::RunningConnection(int socket_, AbstractEngine& engine_)
    : socket(socket_ ), engine(engine_)
{
    th = std::thread(&RunningConnection::start, this);
}

void RunningConnection::start() {
    char buffer[512];
    std::string output;
    int result;

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
        
        spdlog::info(">> {}: received command: {}", socket, buffer);

        OpResult opres;
        if (buffer[0] == 'G') {  // GET
            char kl = buffer[1];
            std::string k(buffer+2, kl);
            opres = engine.get(k);
        }
        else if (buffer[0] == 'S') {
            char kl = buffer[1];
            char vl = buffer[2+kl];
            
            std::string k(buffer+2, kl);
            std::string v(buffer+2+kl+1, vl);
            opres = engine.set(k, v);
        }
        else if (buffer[0] == 'D') {
            char kl = buffer[1];
            std::string k(buffer+2, kl);
            opres = engine.drop(k);
        } else {
            continue;
        }

        memset(buffer, 0, 500);
        if (opres.success) {
            buffer[0] = 'O';
            buffer[1] = 'K';
            if (opres.value.has_value()) {
                auto& v = opres.value.value();
                buffer[2] = (char) v.size();
                strcpy(buffer+3, v.c_str());
            }
        } else {
            buffer[0] = 'E';
            buffer[1] = 'R';
            if (opres.error_msg.has_value()) {
                auto& v = opres.error_msg.value();
                buffer[2] = (char) v.size();
                strcpy(buffer+3, v.c_str());
            }
        }
        

        spdlog::info(">> {}: Sending back: {}", socket, buffer);

        send(socket, buffer, strlen(buffer), 0);
    }

    close(socket);
}