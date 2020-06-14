#include <string>
#include <cstring>
#include <variant>

#include "catch2/catch.hpp"

#include "common/messages.hpp"
#include "common/serialization.hpp"

using namespace std::literals::string_literals;


TEST_CASE( "Unpack GET request" ) {
    char packed[] = { GET_REQ,
        5, 0, 0, 0, 0, 0, 0, 0,  // 8 byte number, little endian
        '1', '2', '3', '4', '5', 1, 2, 3, 4 // NOTE: some worthless chars at the end
    };
    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MessageGetRequest&>(*unpacked);

    REQUIRE(msg.get_flag() == GET_REQ);
    REQUIRE(msg.key == "12345");
}

TEST_CASE( "Unpack GET response " ) {
    char packed[] = { GET_RESP, 1, 0, 0, 0, 0, 0, 0, 0, 'X' };
        
    auto unpacked = Message::unpack_message(std::string(packed, 10));
    auto msg = dynamic_cast<MessageGetResponse&>(*unpacked);

    REQUIRE(msg.get_flag() == GET_RESP);
    REQUIRE(std::holds_alternative<std::string>(msg.value));
    REQUIRE(std::get<std::string>(msg.value) == "X");
}

TEST_CASE( "Unpack GET response without value" ) {
    unsigned char packed[] = { GET_RESP,  255, 255, 255, 255, 255, 255, 255, 255, 2, 3, 4 }; // Only first char needed, other is worthless
        
    std::string packed_str(reinterpret_cast<char*>(packed), 12);
    auto unpacked = Message::unpack_message(packed_str);
    auto msg = dynamic_cast<MessageGetResponse&>(*unpacked);

    REQUIRE(msg.get_flag() == GET_RESP);
    REQUIRE(std::holds_alternative<tomb>(msg.value));
}

TEST_CASE( "Unpack GET OK response with zero len" ) {
    char packed[] = { GET_RESP,
        0, 0, 0, 0, 0, 0, 0, 0,  // 0 len of string, means empty
        'a', 'c'  // some worthless at the end
    }; 
        
    auto unpacked = Message::unpack_message(std::string(packed, 11));
    auto msg = dynamic_cast<MessageGetResponse&>(*unpacked);

    REQUIRE(msg.get_flag() == GET_RESP);
    REQUIRE(std::holds_alternative<std::string>(msg.value));
    REQUIRE(std::get<std::string>(msg.value) == "");

    std::string packed_back = msg.pack_message();

    REQUIRE(0 == strncmp(packed, packed_back.c_str(), 9));
}

TEST_CASE( "Pack SET request" ) {
    MessageSetRequest m;
    m.key = "Some Key";
    m.value = "\n\t\0 "s;

    char expected[] = { SET_REQ,
        8, 0, 0, 0, 0, 0, 0, 0,
        'S', 'o', 'm', 'e', ' ', 'K', 'e', 'y',
        4, 0, 0, 0, 0, 0, 0, 0,
        '\n', '\t', '\0', ' '
    };

    auto s = m.pack_message();

    REQUIRE(s == std::string(expected, 29));
}

TEST_CASE( "Pack SET request without value" ) {
    MessageSetRequest m;
    m.key = "Some Key";
    m.value = tomb::create();

    unsigned char expected[] = { SET_REQ,
        8, 0, 0, 0, 0, 0, 0, 0,
        'S', 'o', 'm', 'e', ' ', 'K', 'e', 'y',
        255, 255, 255, 255, 255, 255, 255, 255
    };

    auto s = m.pack_message();
    std::string expected_str(reinterpret_cast<char*>(expected), 25);
    REQUIRE(s == expected_str);
}

TEST_CASE( "Unpack SET request with zero len" ) {

    char packed[] = { SET_REQ,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };

    
    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MessageSetRequest&>(*unpacked);

    REQUIRE(msg.get_flag() == SET_REQ);
    REQUIRE(msg.key == "");
    REQUIRE(std::holds_alternative<std::string>(msg.value));
    REQUIRE(std::get<std::string>(msg.value) == "");

    std::string packed_back = msg.pack_message();

    REQUIRE(0 == strncmp(packed, packed_back.c_str(), 17));
    
}

TEST_CASE( "Unpack SET request without value (a.k.a DROP) " ) {

    unsigned char packed[] = { SET_REQ,
        3, 0, 0, 0, 0, 0, 0, 0,
        'k', 'e', 'y',
        255, 255, 255, 255, 255, 255, 255, 255
    };

    std::string packed_str(reinterpret_cast<char*>(packed), 20);
    auto unpacked = Message::unpack_message(packed_str);
    auto msg = dynamic_cast<MessageSetRequest&>(*unpacked);

    REQUIRE(msg.get_flag() == SET_REQ);
    REQUIRE(msg.key == "key");
    REQUIRE(std::holds_alternative<tomb>(msg.value));

    std::string packed_back = msg.pack_message();

    REQUIRE(0 == strncmp(reinterpret_cast<char*>(packed), packed_back.c_str(), 20));
    
}











