#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <optional>
#include <memory>

#include "catch2/catch.hpp"

#include "common/messages.hpp"
#include "network/socket_server.hpp"
#include "engine/interface.hpp"
#include "lib/aroww.hpp"

using namespace std::literals::string_literals;


class Call {
public:
    std::string type;
    std::string key;
    std::optional<string_or_tomb> value;
    Call(std::string t, std::string k, std::optional<string_or_tomb> v): type(t), key(k), value(v) {};
};

class DummyTestEngine: public AbstractEngine {
public:
    std::vector<Call> received;
    std::unique_ptr<Message> next_response;
    
    std::unique_ptr<Message> get(std::string key) {
        received.emplace_back("get", key, std::nullopt);
        return std::move(next_response);
    }
    std::unique_ptr<Message> set(std::string key, std::string value) {
        received.emplace_back("set", key, value);
        return std::move(next_response);
    }
    std::unique_ptr<Message> drop(std::string key) {
        received.emplace_back("drop", key, tomb::create());
        return std::move(next_response);
    }
};


class TestServer {
public:
    DummyTestEngine engine;
    std::unique_ptr<SimpleSocketServer> _socket_server;
    std::thread _server_thread;
    TestServer() {
        engine.next_response = nullptr;

        _socket_server = std::make_unique<SimpleSocketServer>(7333, engine);
        _server_thread = std::thread(&SimpleSocketServer::start_listening, _socket_server.get());
    }
    ~TestServer() {

        _socket_server->send_close_signal();
        _socket_server->await_closing();
        _server_thread.join();  // ensure thread is finished
    }
};


TEST_CASE( "Send and receive some messages" ) {
    TestServer server;

    ArowwDB db{"localhost", "7333"};
    
    // Set value and check response
    {
        server.engine.next_response = std::make_unique<MessageSetResponse>();
        auto resp = db.set("first", "some\n\t\0  key \0\n"s);
        REQUIRE(resp->get_flag() == SET_RESP);

        Call p = server.engine.received[0];
        REQUIRE(p.type == "set");
        REQUIRE(p.key == "first");
        REQUIRE(std::get<std::string>(p.value.value()) == "some\n\t\0  key \0\n"s);
    }
    
    
    // Get value
    {
        MessageGetResponse* msg = new MessageGetResponse();
        msg->value = "Hey there!";
        server.engine.next_response = std::unique_ptr<Message>(msg);
        
        auto resp = db.get("");
        REQUIRE(resp->get_flag() == GET_RESP);
        auto resp_casted = dynamic_cast<MessageGetResponse*>(resp.get());
        REQUIRE(std::get<std::string>(resp_casted->value) == "Hey there!");

        Call p = server.engine.received[1];
        REQUIRE(p.type == "get");
        REQUIRE(p.key == "");

    }
    
}
