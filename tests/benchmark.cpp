#include <string>
#include <cstring>
#include <filesystem>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include "tests_config.hpp"

#include "engine/engine.hpp"



TEST_CASE( "Benchmark engine" ) {
    if (std::filesystem::exists(TEMP_DIR)) {
        std::filesystem::remove_all(TEMP_DIR);
    }
    std::filesystem::create_directory(TEMP_DIR);

    EngineConfiguration conf{TEMP_DIR};
    DBEngine engine{conf};


    BENCHMARK ("set 100 values") {
        for (int i = 0; i < 100; i++) {
            auto resp = engine.set(std::to_string(i), "xxxxxxxxx");
        }
        return 0;
    };

    BENCHMARK ("set 200 values, bigger") {
        for (int i = 0; i < 200; i++) {
            auto resp = engine.set(std::to_string(i), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
        }
        return 0;
    };
    
        
}