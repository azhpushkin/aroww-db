#include <string>
#include <optional>
#include <filesystem>
#include <variant>
#include <random>

#include "catch2/catch.hpp"
#include "fmt/format.h"

#include "engine/engine.hpp"
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
    REQUIRE (std::holds_alternative<std::nullptr_t>(res.value())); \
}


#define REQUIRE_HAS_KEY_AND_EQUALS(seg, key, expected_value) { \
    auto res = seg->lookup(key); \
    REQUIRE (res.has_value()); \
    REQUIRE (std::holds_alternative<std::string>(res.value())); \
    REQUIRE (std::get<std::string>(res.value()) == expected_value);  \
}


TEST_CASE( "dump_memtable" ) {
    MemTable memtable;

    memtable["empty_key"] = std::nullopt;
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





TEST_CASE( "merge segments " ) {
    MemTable memtable1;
    memtable1["zero"] = random_string(20);
    memtable1["one"] = std::nullopt;
    memtable1["two_empty"] = std::nullopt;
    memtable1["three"] = "33333\b\b33333";
    auto index_step1 = GENERATE(as<unsigned int>{}, 1, 4);
    auto segment1 = Segment::dump_memtable(memtable1, TEMP_DIR, 111, index_step1);

    MemTable memtable2;
    memtable2["five"] = random_string(20);
    memtable2["six"] = random_string(30);
    memtable2["zero"] = std::nullopt;
    memtable2["11"] = random_string(30);
    memtable2["22"] = random_string(30);
    memtable2["33"] = random_string(30);
    memtable2["44"] = random_string(30);
    memtable2["55"] = random_string(30);
    memtable2["66"] = random_string(30);
    auto index_step2 = GENERATE(as<unsigned int>{}, 1, 2);
    auto segment2 = Segment::dump_memtable(memtable2, TEMP_DIR, 222, index_step2);

    MemTable memtable3;
    memtable3["44"] = "overwritten";
    memtable3["one"] = "hey there";
    memtable3["55"] = std::nullopt;
    auto index_step3 = GENERATE(as<unsigned int>{}, 2, 3);
    auto segment3 = Segment::dump_memtable(memtable3, TEMP_DIR, 333, index_step3);
    

    std::vector<SegmentPtr> to_merge = {segment1, segment2, segment3};

    auto result_step = GENERATE(as<unsigned int>{}, 1, 3, 5);
    auto merged_segment = Segment::merge(to_merge, result_step);


    SECTION(fmt::format(
        "index sizes: <s1:{}>, <s2:{}>, <s3:{}>, <res:{}> ",
        index_step1, index_step2, index_step3, result_step
    )) {
        // Left after segment 1
        REQUIRE_HAS_KEY_TOMB(merged_segment, "two_empty");
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "three", memtable1["three"]);

        // Left after segment 2
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "five", memtable2["five"]);
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "six", memtable2["six"]);
        REQUIRE_HAS_KEY_TOMB(merged_segment, "zero");
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "11", memtable2["11"]);
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "22", memtable2["22"]);
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "33", memtable2["33"]);
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "66", memtable2["66"]);

        // Left after segment 3
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "44", memtable3["44"]);
        REQUIRE_HAS_KEY_AND_EQUALS(merged_segment, "one", memtable3["one"]);
        REQUIRE_HAS_KEY_TOMB(merged_segment, "55");
    }
}

