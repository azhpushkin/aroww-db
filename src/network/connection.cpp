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
    char buffer[1024];
    std::string output;
    int bytes_read;

    spdlog::info(">> {}: thread started", socket);
    while (1) {
        bytes_read = recv(socket, buffer, 512, 0);
        if (bytes_read == 0) {
            spdlog::info(">> {}: connection closed", socket);
            break;
        }
        if (bytes_read == -1) {
            perror("Error receiving from connection!");
            break;
        }
        
        auto req = Message::unpack_message(std::move(std::string(buffer, bytes_read)));
        spdlog::info(">> {}: received command: {}", socket, req->get_flag());
        

        std::unique_ptr<Message> resp;
        if (auto p = dynamic_cast<MsgGetReq*>(req.get())) {
            resp = engine.get(p->key);
        } else if (auto p = dynamic_cast<MsgSetReq*>(req.get())) {
            resp = engine.set(p->key, p->value);
        }else if (auto p = dynamic_cast<MsgDropReq*>(req.get())) {
            resp = engine.drop(p->key);
        } else {
            auto err = new MsgErrorResp();
            err->error_msg = fmt::format("Not supported operation: {}", req->get_flag());
            resp = std::unique_ptr<Message>(err);
        }

        std::string resp_buf = resp->pack_message();
        spdlog::info(">> {}: Sending back: {}", socket, resp->get_flag());

        send(socket, resp_buf.c_str(), resp_buf.size(), 0);
    }

    close(socket);
}
