#include <cerrno>
#include <sys/socket.h>

#include <thread>

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#include "socket_server.hpp"
#include "engine/engine.hpp"

#include "commands.pb.h"


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
            opres = engine.get(command.key());
            if (opres.success) {
                result.set_type(ResultType::OK);
                result.set_value(opres.value.value());
            } else {
                result.set_type(ResultType::KEY_MISSING);
                result.set_error_msg(opres.error_msg.value());
            }
        }
        if (command.type() == 2) {
            opres = engine.set(command.key(), command.value());
            result.set_type(ResultType::OK);
        }
        if (command.type() == 3) {
            opres = engine.drop(command.key());
            result.set_type(ResultType::OK);
        }
        
        output.clear();
        result.SerializeToString(&output);
        spdlog::info(">> {}: Sending back: {}(key={},value={})", socket, result.type(), command.key(), command.value());

        send(socket, output.c_str(), output.length(), 0);
    }

    close(socket);
}