#include <filesystem>

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#define CATCH_CONFIG_RUNNER 1
#define CATCH_CONFIG_ENABLE_BENCHMARKING 1
#include "catch2/catch.hpp"

#include "tests_config.hpp"


namespace fs = std::filesystem;


int main( int argc, char* argv[] ) {
    // StartUp
    if (fs::exists(TEMP_DIR)) {
        fs::remove_all(TEMP_DIR);
    }
    fs::create_directory(TEMP_DIR);
    spdlog::set_level(spdlog::level::warn);  // Avoid output 

    // Run actual tests
    int result = Catch::Session().run( argc, argv );

    // TearDown
    fs::remove_all(TEMP_DIR);

    return result;
}