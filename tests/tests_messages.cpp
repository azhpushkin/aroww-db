#include <string>
#include <cstring>

#include "catch2/catch.hpp"

#include "network/messages.hpp"

using namespace std::literals::string_literals;


TEST_CASE( "Unpack GET request" ) {
    char packed[] = { GetReqType,
        5, 0, 0, 0, 0, 0, 0, 0,  // 8 byte number, little endian
        '1', '2', '3', '4', '5', 1, 2, 3, 4 // NOTE: some harbage at the end
    };
    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MsgGetReq&>(*unpacked);

    REQUIRE(msg.key == "12345");
}

TEST_CASE( "Pack SET request" ) {
    MsgSetReq m;
    m.key = "Some Key";
    m.value = "\n\t\0 "s;

    char expected[] = { SetReqType,
        8, 0, 0, 0, 0, 0, 0, 0,
        'S', 'o', 'm', 'e', ' ', 'K', 'e', 'y',
        4, 0, 0, 0, 0, 0, 0, 0,
        '\n', '\t', '\0', ' '
    };

    auto s = m.pack_message();

    REQUIRE(s == std::string(expected, 29));
}


TEST_CASE( "Unpack SET request with zero len" ) {

    char packed[] = { SetReqType,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };

    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MsgSetReq&>(*unpacked);

    REQUIRE(msg.get_flag() == SetReqType);
    REQUIRE(msg.key == "");
    REQUIRE(msg.value == "");

    std::string packed_back = msg.pack_message();

    REQUIRE(0 == strncmp(packed, packed_back.c_str(), 17));
    
}


TEST_CASE( "Unpack GET MISSING response" ) {
    char packed[] = { GetMissingRespType, 1, 2, 3, 4 }; // Only 1 char needed, other is garbage
        
    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MsgGetMissingResp&>(*unpacked);

    REQUIRE(msg.get_flag() == GetMissingRespType);
}


TEST_CASE( "Unpack GET OK response with zero len" ) {
    char packed[] = { GetOkRespType,
        0, 0, 0, 0, 0, 0, 0, 0,  // 0 len of string, means empty
        'a', 'c'  // some garbage at the end
    }; 
        
    auto unpacked = Message::unpack_message(std::string(packed, 18));
    auto msg = dynamic_cast<MsgGetOkResp&>(*unpacked);

    REQUIRE(msg.get_flag() == GetOkRespType);
    REQUIRE(msg.val == "");

    std::string packed_back = msg.pack_message();

    REQUIRE(0 == strncmp(packed, packed_back.c_str(), 9));
}


