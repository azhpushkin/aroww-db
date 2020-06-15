#include <thread>
#include <string>
#include <iostream>
#include <cstring>
#include <optional>
#include <memory>

#include "catch2/catch.hpp"

#include "common/messages.hpp"
#include "common/string_or_tomb.hpp"
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
    std::unique_ptr<Call> last_received;
    std::unique_ptr<Message> next_response;
    
    std::unique_ptr<Message> get(std::string key) {
        last_received = std::unique_ptr<Call>(new Call("get", key, std::nullopt));
        return std::move(next_response);
    }
    std::unique_ptr<Message> set(std::string key, std::string value) {
        last_received = std::unique_ptr<Call>(new Call("set", key, value));
        return std::move(next_response);
    }
    std::unique_ptr<Message> drop(std::string key) {
        last_received = std::unique_ptr<Call>(new Call("drop", key, tomb::create()));
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
        engine.last_received = nullptr;

        _socket_server = std::make_unique<SimpleSocketServer>(7333, engine);
        _server_thread = std::thread(&SimpleSocketServer::start_listening, _socket_server.get());
    }
    ~TestServer() {

        _socket_server->send_close_signal();
        _socket_server->await_closing();
        _server_thread.join();  // ensure thread is finished
    }
};


TEST_CASE( "Basic commands " ) {
    TestServer server;
    aroww::ArowwDB db{"localhost", "7333"};
    
    SECTION ("Simple SET") {
        server.engine.next_response = std::make_unique<MessageSetResponse>();
        db.set("first", "some\n\t\0  key \0\n"s);

        auto received = server.engine.last_received.get();
        REQUIRE(received->type == "set");
        REQUIRE(received->key == "first");
        REQUIRE(std::get<std::string>(received->value.value()) == "some\n\t\0  key \0\n"s);
    }

    SECTION ("Simple DROP") {
        server.engine.next_response = std::make_unique<MessageSetResponse>();
        db.drop("value_to_drop");

        auto received = server.engine.last_received.get();
        REQUIRE(received->type == "drop");
        REQUIRE(received->key == "value_to_drop");
    }

    SECTION ("Simple GET with return value") {
        MessageGetResponse* msg = new MessageGetResponse();
        msg->value = "Hey there!";
        server.engine.next_response = std::unique_ptr<Message>(msg);
        
        auto resp = db.get("");
        REQUIRE(resp.has_value());
        REQUIRE(resp.value() == "Hey there!");

        auto received = server.engine.last_received.get();
        REQUIRE(received->type == "get");
        REQUIRE(received->key == "");
    }

    SECTION ("Simple GET for missing key ") {
        MessageGetResponse* msg = new MessageGetResponse();
        msg->value = tomb::create();
        server.engine.next_response = std::unique_ptr<Message>(msg);
        
        auto resp = db.get("missing_key");
        REQUIRE(!resp.has_value());

        auto received = server.engine.last_received.get();
        REQUIRE(received->type == "get");
        REQUIRE(received->key == "missing_key");
    }
    
}


TEST_CASE( "Exceptions handling " ) {
    TestServer server;
    aroww::ArowwDB db{"localhost", "7333"};
    
    SECTION ("Requested GET, SET returned") {
        server.engine.next_response = std::make_unique<MessageSetResponse>();
        REQUIRE_THROWS_MATCHES(
            db.get("first"),
            aroww::ArowwException,
            Catch::Message("Wrong message received: expected g, got s!")); 

    }

    SECTION ("Requested SET, GET returned") {
        server.engine.next_response = std::make_unique<MessageGetResponse>();
        REQUIRE_THROWS_MATCHES(
            db.set("first", "1"),
            aroww::ArowwException,
            Catch::Message("Wrong message received: expected s, got g!")); 
    }

    SECTION ("Requested GET, GET request type returned") {
        server.engine.next_response = std::make_unique<MessageGetRequest>();
        REQUIRE_THROWS_MATCHES(
            db.get("first"),
            aroww::ArowwException,
            Catch::Message("Wrong message received: expected g, got G!")); 
    }

    SECTION ("Error message returned") {
        MessageErrorResponse* msg = new MessageErrorResponse();
        msg->error_msg = "Custom error message";
        server.engine.next_response = std::unique_ptr<MessageErrorResponse>(msg);

        REQUIRE_THROWS_MATCHES(
            db.get("first"),
            aroww::ArowwException,
            Catch::Message("Custom error message")); 
    }
}
