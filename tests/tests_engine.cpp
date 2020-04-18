#include "catch2/catch.hpp"

#include "engine/engine.hpp"

SimpleInMemoryEngine engine{};

TEST_CASE( "Test empty get" ) {
    REQUIRE(engine.get("MissingKey") == OpResult{ false, std::nullopt, "Key missing"});
}
    
TEST_CASE( "Test set" ) {
    REQUIRE(engine.set("NewKey", "112211") == OpResult{ true, std::nullopt, std::nullopt});
}

TEST_CASE( "Test drop" ) {
    REQUIRE(engine.drop("MissingKey") == OpResult{ true, std::nullopt, std::nullopt});
}

TEST_CASE( "Test get after set" ) {
    engine.set("SomeKey", "112211");
    REQUIRE(engine.get("SomeKey") == OpResult{ true, "112211", std::nullopt});
}

TEST_CASE( "Test get after set and drop" ) {
    engine.set("SomeKey", "112211");
    engine.drop("SomeKey");
    REQUIRE(engine.get("SomeKey") == OpResult{ false, std::nullopt, "Key missing"});
}
