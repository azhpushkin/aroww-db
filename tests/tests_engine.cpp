#include <filesystem>
#include <fstream>

#include "catch2/catch.hpp"

#include "engine/engine.hpp"

bool operator==(const OpResult& lhs, const OpResult& rhs) {
    return (
        lhs.success == rhs.success
        and lhs.error_msg == rhs.error_msg
        and lhs.value == rhs.value
    );
}
bool operator!=(const OpResult& lhs, const OpResult& rhs) {
    return !(lhs==rhs);
}
std::ostream& operator << ( std::ostream& os, OpResult const& opres ) {
    os << (opres.success ? "OK." : "FAIL.");
    os << (opres.value.has_value() ? opres.value.value() : "<nullopt>");
    os << (opres.error_msg.has_value() ? opres.error_msg.value() : "<nullopt>");
    return os;
}


TEST_CASE( "Empty test" ) {
    REQUIRE(OpResult{true, "val", "err"} == OpResult{true, "val", "err"});
}


class TempDirFixture {
protected:
    std::filesystem::path temp_dir;
public:
    TempDirFixture() {
        temp_dir = std::filesystem::temp_directory_path() / ".arrow_test_tmp";
        std::filesystem::create_directory(temp_dir);
    }
    ~TempDirFixture() {
        std::filesystem::remove_all(temp_dir);
    }
};


TEST_CASE_METHOD (TempDirFixture, "Log file contents" ) {
    SingleFileLogEngine engine(temp_dir);

    engine.set("key1", "First");
    engine.set("key2", "Second");
    engine.drop("key1");
    engine.drop("key3");
    engine.set("key2", "Second_v2");
    engine.set("key3", "THIRD");

    std::vector<std::string> contents;
    std::string temp;
    std::ifstream file(temp_dir / "db.txt");
    while(std::getline(file, temp, '\n')) {
        contents.push_back(temp);
    }

    REQUIRE(contents.size() == 6);
    REQUIRE(contents.at(0) == "key1\vFirst");
    REQUIRE(contents.at(1) == "key2\vSecond");
    REQUIRE(contents.at(2) == "key1\v");
    REQUIRE(contents.at(3) == "key3\v");
    REQUIRE(contents.at(4) == "key2\vSecond_v2");
    REQUIRE(contents.at(5) == "key3\vTHIRD");
}


TEST_CASE_METHOD (TempDirFixture, "Functional test" ) {
    SingleFileLogEngine engine(temp_dir);

    // Simple set and re-set
    REQUIRE( engine.get("111") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.set("111", "value1") == OpResult{true, std::nullopt, std::nullopt});
    REQUIRE( engine.get("111") == OpResult{true, "value1", std::nullopt});
    REQUIRE( engine.set("111", "value2") == OpResult{true, std::nullopt, std::nullopt});
    REQUIRE( engine.get("111") == OpResult{true, "value2", std::nullopt});

    // Check similar values
    REQUIRE( engine.get("1111") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.get("11") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.get("111v") == OpResult{false, std::nullopt, "Key missing"});
    
    // Set other value and check initial too
    REQUIRE( engine.get("222") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.set("222", "other") == OpResult{true, std::nullopt, std::nullopt});
    REQUIRE( engine.get("111") == OpResult{true, "value2", std::nullopt});
    REQUIRE( engine.get("222") == OpResult{true, "other", std::nullopt});

    // drop
    REQUIRE( engine.drop("111") == OpResult{true, std::nullopt, std::nullopt});
    REQUIRE( engine.drop("333") == OpResult{true, std::nullopt, std::nullopt});
    REQUIRE( engine.get("111") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.get("222") == OpResult{true, "other", std::nullopt});
}


TEST_CASE_METHOD (TempDirFixture, "Init from file" ) {    
    std::ofstream file(temp_dir / "db.txt");

    file << "key1\vFirst" << std::endl;
    file << "key2\vSecond" << std::endl;
    file << "key1\v" << std::endl;
    file << "key3\v" << std::endl;
    file << "key2\vSecond_v2" << std::endl;
    file << "key3\vTHIRD" << std::endl;
    

    SingleFileLogEngine engine(temp_dir);
    REQUIRE( engine.get("key1") == OpResult{false, std::nullopt, "Key missing"});
    REQUIRE( engine.get("key2") == OpResult{true, "Second_v2", std::nullopt});
    REQUIRE( engine.get("key3") == OpResult{true, "THIRD", std::nullopt});

}
