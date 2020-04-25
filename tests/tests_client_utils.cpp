#include "catch2/catch.hpp"

#include <sstream>
#include <iostream>

#include "utils/client_utils.hpp"


TEST_CASE( "Test split single arg" ) {
    auto res = split_by_whitespaces("hello");
    REQUIRE(res.size() == 1);
    REQUIRE(res.at(0) == "hello");
}


TEST_CASE( "Test split three args" ) {
    auto res = split_by_whitespaces("hello world there");
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "hello");
    REQUIRE(res.at(1) == "world");
    REQUIRE(res.at(2) == "there");
}

TEST_CASE( "Test split with several delimeters" ) {
    auto res = split_by_whitespaces("hello     world");
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "hello");
    REQUIRE(res.at(1) == "world");
}

TEST_CASE( "Test split complex case" ) {
    auto res = split_by_whitespaces("hello     world how are  ,  you");
    REQUIRE(res.size() == 6);
    REQUIRE(res.at(0) == "hello");
    REQUIRE(res.at(1) == "world");
    REQUIRE(res.at(2) == "how");
    REQUIRE(res.at(3) == "are");
    REQUIRE(res.at(4) == ",");
    REQUIRE(res.at(5) == "you");
}

TEST_CASE( "Test split empty" ) {
    auto res = split_by_whitespaces("");
    REQUIRE(res.size() == 0);
}


TEST_CASE( "Test parse_input" ) {
    auto res = parse_input("HellO world UPPER!");
    REQUIRE(res.name == "hello");
    REQUIRE(res.args.size() == 2);
    REQUIRE(res.args.at(0) == "world");
    REQUIRE(res.args.at(1) == "UPPER!");
}



TEST_CASE( "Test get_command get" ) {
    std::stringstream in{"get 123"};
    std::stringstream out{};
    Command res = get_command(out, in);

    REQUIRE(out.str() == "cli> ");
    REQUIRE(res.name == "get");
    REQUIRE(res.args.size() == 1);
    REQUIRE(res.args.at(0) == "123");
}



TEST_CASE( "Test get_command skip" ) {
    std::stringstream in{"  "};
    std::stringstream out{};
    Command res = get_command(out, in);

    REQUIRE(out.str() == "cli> ");
    REQUIRE(res.name == "skip");
    REQUIRE(res.args.size() == 0);
}

TEST_CASE( "Test get_command error" ) {
    std::stringstream in{"hey there you"};
    std::stringstream out{};
    Command res = get_command(out, in);

    REQUIRE(out.str() == "cli> ");
    REQUIRE(res.name == "error");
    REQUIRE(res.args.size() == 1);
}


TEST_CASE( "Test get_command set" ) {
    std::stringstream in{"SET KEY VaLuE"};
    std::stringstream out{};
    Command res = get_command(out, in);

    REQUIRE(out.str() == "cli> ");
    REQUIRE(res.name == "set");
    REQUIRE(res.args.size() == 2);
    REQUIRE(res.args.at(0) == "KEY");
    REQUIRE(res.args.at(1) == "VaLuE");
}