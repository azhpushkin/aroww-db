#include <string>
#include <cstring>

#include "catch2/catch.hpp"

#include "network/messages.hpp"

char* getstr(std::string x) {
    return (char*)(x.c_str());
}

void test_req(std::string&& name, Req&& req, char* expected) {
    SECTION(name) {
        char* buf = pack_request(&req);
        for (size_t i = 0; i < strlen(expected); i++)  {
            REQUIRE(buf[i] == expected[i]);
        }
        free(buf);

        Req* req_unpacked = unpack_request(expected);

        REQUIRE(req_unpacked->type == req.type);
        
        REQUIRE(req_unpacked->key_len == req.key_len);
        REQUIRE(req_unpacked->value_len == req.value_len);

        REQUIRE(strncmp(req_unpacked->key, req.key, req.key_len) == 0);
        REQUIRE(strncmp(req_unpacked->value, req.value, req.value_len) == 0);
        
        free_request(req_unpacked);

    }
}


void test_resp(std::string&& name, Resp&& resp, char* expected) {
    SECTION(name) {
        char* buf = pack_response(&resp);
        for (size_t i = 0; i < strlen(expected); i++)  {
            REQUIRE(buf[i] == expected[i]);
        }
        free(buf);

        Resp* resp_unpacked = unpack_response(expected);

        REQUIRE(resp_unpacked->type == resp.type);
        
        REQUIRE(resp_unpacked->data_len == resp.data_len);
        REQUIRE(strncmp(resp_unpacked->data, resp.data, resp.data_len) == 0);
        
        free_response(resp_unpacked);

    }
}

TEST_CASE( "Request pack and unpack" ) {
    char ex0[] = { GET,
        5, '1', '2', '3', '4', '5', 0
    };
    test_req(
        "GET",
        Req {GET, 5, getstr("123456789"), 0, NULL}, // NOTE: only 5 chars saved
        ex0
    );
    
    char ex1[] = { SET,
        12, '1', '\t', '2', '3', '4', '5', '6', '7', '8', '9', '\v', '0',
        7, 'a', 's', 'd', '\n', 'q', 'w', 'e'
    };
    test_req(
        "SET",
        Req {SET, 12, getstr("1\t23456789\v0"), 7, getstr("asd\nqwe")},
        ex1
    );
}

TEST_CASE( "Response pack and unpack" ) {
    char ex0[] = { GET_OK,
        5, '1', '2', '3', '4', '5'
    };
    test_resp(
        "GET_OK",
        Resp {GET, 5, getstr("123456789")}, // NOTE: only 5 chars saved
        ex0
    );
    
    char ex1[] = { GET_MISSING, 0};
    test_resp(
        "GET_MISSING",
        Resp {GET_MISSING, 0, NULL},
        ex1
    );

    
}

