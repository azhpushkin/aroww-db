#include <string>
#include <cstring>
#include <filesystem>

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include "tests_config.hpp"
#include "fmt/format.h"

#include "engine/engine.hpp"

void cleanup() {
    if (std::filesystem::exists(TEMP_DIR)) {
        std::filesystem::remove_all(TEMP_DIR);
    }
    std::filesystem::create_directory(TEMP_DIR);
}

TEST_CASE( "Basic benchmark" ) {
    EngineConfiguration conf{TEMP_DIR};
    conf.max_memtable_size = 3;
    conf.index_step = 1;

    cleanup();
    DBEngine engine{conf};
    BENCHMARK ("Few-letters key and value") {
        for (int i = 0; i < 300; i++) {
            auto resp = engine.set(std::to_string(i), "1234567890");
        }
        return 0;
    };

}

TEST_CASE( "Benchmark with changing memtable size" ) {
    for (unsigned int i = 5; i < 50; i *= 2) {
        EngineConfiguration conf{TEMP_DIR};
        conf.max_memtable_size = i;
        cleanup();
        DBEngine engine{conf};
        
        BENCHMARK (fmt::format("Max memtable size is {}", i)) {
            for (int i = 0; i < 200; i++) {
                auto resp = engine.set(std::string(20, i), "12345678901234567890123456789012345678901234567890");
            }
            return 0;
        };
    }

    
}
