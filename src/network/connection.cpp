#include <cerrno>
#include <sys/socket.h>
#include <string>
#include <poll.h>
#include <thread>

#include "socket_server.hpp"
#include "engine/interface.hpp"
#include "network/messages.hpp"
#include "utils/logger.hpp"


RunningConnection::RunningConnection(int socket_, AbstractEngine& engine_)
    : socket(socket_ ), engine(engine_)
{
    close_scheduled = false;
    th = std::thread(&RunningConnection::start, this);
}
void RunningConnection::close_conn() {
    close_scheduled = true;
    th.join();
}

void RunningConnection::start() {
    char buffer[1024];
    std::string output;
    int bytes_read;

    struct pollfd *pfds = (struct pollfd *)malloc(sizeof(*pfds) * 1);
    pfds[0].fd = socket;
    pfds[0].events = POLLIN;
    
    log_debug(">> {}: thread started", socket);
    while (!close_scheduled) {
        int poll_count = poll(pfds, 1, 1000);  // 1000 ms
        if (poll_count == -1) {
            perror("poll");
            break;
        }

        if (poll_count == 0) {
            continue;  // timeout
        }

        bytes_read = recv(socket, buffer, 512, 0);
        if (bytes_read == 0) {
            log_debug(">> {}: connection closed", socket);
            break;
        }
        if (bytes_read == -1) {
            perror("Error receiving from connection!");
            break;
        }
        
        auto req = Message::unpack_message(std::move(std::string(buffer, bytes_read)));
        log_debug(">> {}: received command: {}", socket, req->get_flag());
        

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
        log_debug(">> {}: Sending back: {}", socket, resp->get_flag());

        send(socket, resp_buf.c_str(), resp_buf.size(), 0);
    }

    close(socket);
}
