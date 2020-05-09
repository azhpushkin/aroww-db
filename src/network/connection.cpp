#include <cerrno>
#include <sys/socket.h>
#include <string>

#include <thread>

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#include "socket_server.hpp"
#include "engine/interface.hpp"
#include "network/messages.hpp"


RunningConnection::RunningConnection(int socket_, AbstractEngine& engine_)
    : socket(socket_ ), engine(engine_)
{
    th = std::thread(&RunningConnection::start, this);
}

void RunningConnection::start() {
    char buffer[MSG_BUF_SIZE];
    std::string output;
    int result;

    spdlog::info(">> {}: thread started", socket);
    while (1) {
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
        
        Req* req = unpack_request(buffer);
        Resp* resp = alloc_response();
        if (req->type == GET) {  // GET
            std::string k(req->key, req->key_len);
            opres = engine.get(k);
            resp->type = opres.success ? GET_OK : GET_MISSING;
        }
        else if (req->type == SET) {
            std::string k(req->key, req->key_len);
            std::string v(req->value, req->value_len);
            opres = engine.set(k, v);
            resp->type = UPDATE_OK;
        }
        else if (req->type == DROP) {
            std::string k(req->key, req->key_len);
            opres = engine.drop(k);
            resp->type = UPDATE_OK;
        } else {
            opres = {false, std::nullopt, "Unknown command!"};
            resp->type = ERR;
        }

        
        if (opres.value.has_value()) {
            resp->data_len = opres.value.value().size();
            resp->data = strdup(opres.value.value().c_str());
        } else if (opres.error_msg.has_value()) {
            resp->data_len = opres.error_msg.value().size();
            resp->data = strdup(opres.error_msg.value().c_str());
        }
        char* resp_buf = pack_response(resp);
        spdlog::info(">> {}: Sending back: {}", socket);

        send(socket, resp_buf, strlen(resp_buf) + (resp->data_len == 0 ? 1 : 0), 0);
        free_response(resp);
        free_request(req);
    }

    close(socket);
}