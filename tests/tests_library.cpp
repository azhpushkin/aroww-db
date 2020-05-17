#include <string>
#include <iostream>
#include <thread>

#include "catch2/catch.hpp"

#include "network/socket_server.hpp"
#include "engine/interface.hpp"
#include "lib/aroww.hpp"


class TestEngine: public AbstractEngine {
    std::unique_ptr<Message> get(std::string key) {
        // std::cout << "Got GET" << std::endl;
        return std::make_unique<MsgGetMissingResp>();
    }
    std::unique_ptr<Message> set(std::string key, std::string value) {
        // std::cout << "Got SET" << std::endl;
        return std::make_unique<MsgGetMissingResp>();
    }
    std::unique_ptr<Message> drop(std::string key) {
        // std::cout << "Got DROP" << std::endl;
        return std::make_unique<MsgGetMissingResp>();
    }
};


class TestServerFixture {
protected:
    TestEngine engine;
    SimpleSocketServer* _socket_server;
    std::thread _server_thread;
    
public:
    TestServerFixture() {
        _socket_server = new SimpleSocketServer(7333, engine);
        _server_thread = std::thread(&SimpleSocketServer::start_listening, _socket_server);
        _socket_server->ready_mutex.lock();
    }
    ~TestServerFixture() {
        _socket_server->close();
        _server_thread.join();  // wait until closed
    }
};


TEST_CASE_METHOD( TestServerFixture, "Send and receive some messages" ) {
    ArowwDB* db = aroww_init("localhost", "7333");

    auto res = aroww_set(db, "first", "value");
    aroww_free_result(res);

    auto res2 = aroww_get(db, "first");
    aroww_free_result(res2);

    aroww_close(db);
}
