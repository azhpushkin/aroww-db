cmake_minimum_required(VERSION 3.0)

# Use TESTABLE_HDR and TESTABLE_SRC to use files from src/

include_directories(PUBLIC ${TESTABLE_HDR})
set(TEST_FILES tests_engine.cpp tests_main.cpp)


# Catch2 tests
find_package(Catch2 REQUIRED)
find_package(Catch2 REQUIRED)
add_executable(test ${TEST_FILES} ${TESTABLE_SRC} ${PROTO_SRCS})
target_link_libraries(test Catch2::Catch2)
target_link_libraries(test fmt::fmt)
