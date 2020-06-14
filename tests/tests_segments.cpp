#include <string>
#include <optional>
#include <filesystem>
#include <variant>
#include <random>

#include "catch2/catch.hpp"
#include "fmt/format.h"

#include "engine/engine.hpp"
#include "common/serialization.hpp"
#include "tests_config.hpp"


namespace fs = std::filesystem;

std::random_device rd;

std::string random_string(size_t len)
{
     std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
     std::mt19937 generator(rd());
     std::shuffle(str.begin(), str.end(), generator);

     return str.substr(0, len);    
}


#define REQUIRE_NO_KEY(seg, key) { \
    auto res = seg->lookup(key); \
    REQUIRE_FALSE (res.has_value()); \
}

#define REQUIRE_HAS_KEY_TOMB(seg, key) { \
    auto res = seg->lookup(key); \
    REQUIRE (res.has_value()); \
    REQUIRE (std::holds_alternative<tomb>(res.value())); \
}


#define REQUIRE_HAS_KEY_AND_EQUALS(seg, key, expected_value) { \
    auto res = seg->lookup(key); \
    REQUIRE (res.has_value()); \
    REQUIRE (std::holds_alternative<std::string>(res.value())); \
    REQUIRE (std::get<std::string>(res.value()) == expected_value);  \
}


TEST_CASE( "dump_memtable" ) {
    MemTable memtable;

    tomb t{};
    memtable["empty_key"] = t;
    memtable["first"] = "111111111";
    memtable["second"] = std::string(30, 'x');
    memtable["hello"] = "W orl\n\td";

    
    auto index_step = GENERATE(as<unsigned int>{}, 1, 2, 3, 4);

    auto dumped_seg = Segment::dump_memtable(memtable, TEMP_DIR, static_cast<int64_t>(index_step), index_step);
    // Re-create to make sure everything is correct if load from start
    auto reloaded_seg = std::make_shared<Segment>(dumped_seg->file_path);

    SECTION("index_step of " + std::to_string(index_step)) {
        REQUIRE(dumped_seg->keys_amount == 4);
        REQUIRE(reloaded_seg->keys_amount == 4);
        if (index_step == 1) {
            REQUIRE(dumped_seg->indexed_keys_amount == 4);
            REQUIRE(reloaded_seg->indexed_keys_amount == 4);
        }
        else if (index_step == 2 || index_step == 3) {
            REQUIRE(dumped_seg->indexed_keys_amount == 2);
            REQUIRE(reloaded_seg->indexed_keys_amount == 2);
        }
        else {
            REQUIRE(dumped_seg->indexed_keys_amount == 1);
            REQUIRE(reloaded_seg->indexed_keys_amount == 1);
        }

        REQUIRE_HAS_KEY_TOMB(dumped_seg, "empty_key");
        REQUIRE_HAS_KEY_TOMB(reloaded_seg, "empty_key");

        REQUIRE_HAS_KEY_AND_EQUALS(dumped_seg, "first", "111111111");
        REQUIRE_HAS_KEY_AND_EQUALS(reloaded_seg, "first", "111111111");
        REQUIRE_HAS_KEY_AND_EQUALS(dumped_seg, "second", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
        REQUIRE_HAS_KEY_AND_EQUALS(reloaded_seg, "second", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
        REQUIRE_HAS_KEY_AND_EQUALS(dumped_seg, "hello", "W orl\n\td");
        REQUIRE_HAS_KEY_AND_EQUALS(reloaded_seg, "hello", "W orl\n\td");

        REQUIRE_NO_KEY(dumped_seg, "empty");
        REQUIRE_NO_KEY(reloaded_seg, "orl");
    }
}


