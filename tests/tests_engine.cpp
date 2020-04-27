#include <filesystem>
#include <fstream>
#include <iostream>

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

TEST_CASE( "Empty test" ) {
    REQUIRE(OpResult{true, "val", "err"} == OpResult{true, "val", "err"});
}


class TempDirFixture {
protected:
    std::filesystem::path temp_dir;
public:
    TempDirFixture() {
        temp_dir = std::filesystem::temp_directory_path() / ".arrow_test_tmp";
        std::cout << temp_dir <<std::endl;
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
        std::cout << temp <<std::endl;
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
