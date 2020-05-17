#include <thread>
#include <vector>
#include <string>
#include <iostream>

#include "catch2/catch.hpp"

#include "network/socket_server.hpp"
#include "engine/interface.hpp"
#include "lib/aroww.hpp"


class DummyTestEngine: public AbstractEngine {
public:
    std::vector<std::unique_ptr<Message>> received;
    std::unique_ptr<Message> next_response;
    
    std::unique_ptr<Message> get(std::string key) {
        // MsgGetReq msg;
        // msg.key = key;
        // received.emplace_back(msg);
        return std::move(next_response);
    }
    std::unique_ptr<Message> set(std::string key, std::string value) {
        // MsgSetReq msg;
        // msg.key = key;
        // msg.value = value;
        // received.emplace_back(msg);
        return std::move(next_response);
    }
    std::unique_ptr<Message> drop(std::string key) {
        // MsgDropReq msg;
        // msg.key = key;
        // received.emplace_back(msg);
        return std::move(next_response);
    }
};


class TestServer {
public:
    DummyTestEngine engine;
    SimpleSocketServer* _socket_server;
    std::thread _server_thread;
    TestServer() {
        engine.next_response = nullptr;

        _socket_server = new SimpleSocketServer(7333, engine);
        _server_thread = std::thread(&SimpleSocketServer::start_listening, _socket_server);
        _socket_server->ready_mutex.lock();
    }
    ~TestServer() {
        _socket_server->close();
        _server_thread.join();  // wait until closed
    }
};


TEST_CASE( "Send and receive some messages" ) {
    TestServer server;

    ArowwDB* db = aroww_init("localhost", "7333");
    ArowwResult* res;
    
    // Set value and check response
    server.engine.next_response = std::make_unique<MsgUpdateOkResp>();;
    res = aroww_set(db, "first", "value");
    REQUIRE(res->is_ok == true);
    aroww_free_result(res);
    

    auto next2 = std::make_unique<MsgGetOkResp>();
    next2->val = "Hey there!";
    server.engine.next_response = std::move(next2);

    res = aroww_get(db, "first");
    REQUIRE(res->is_ok == true);
    REQUIRE(std::string(res->value) == "Hey there!");
    aroww_free_result(res);

    aroww_close(db);
}
