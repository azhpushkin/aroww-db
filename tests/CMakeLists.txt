cmake_minimum_required(VERSION 3.0)

# Add TESTABLE_HDR and TESTABLE_SRC to use files from src/
include_directories(PUBLIC ${TESTABLE_HDR_INCLUDE})
set(TEST_FILES
    tests_messages.cpp
    tests_segment.cpp
    tests_library.cpp
    tests_main.cpp
)

find_package(Catch2 REQUIRED)
link_libraries(aroww-db-for-tests)
link_libraries(Catch2::Catch2)

# Catch2 test runner compile
add_executable(test_runner ${TEST_FILES})
set_target_properties(test_runner PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Custom targets to run tests, always rebuild
set(TEST_TARGETS_NAMES test tests catch catch2)

foreach(TARGET_NAME ${TEST_TARGETS_NAMES})
    add_custom_target(${TARGET_NAME} DEPENDS test_runner)
    set_target_properties(${TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND $<TARGET_FILE:test_runner>)
endforeach(TARGET_NAME)

# Benchmark target
# add_executable(bench_runner benchmark.cpp tests_main.cpp)
# set_target_properties(bench_runner PROPERTIES EXCLUDE_FROM_ALL TRUE)

# add_custom_target(bench DEPENDS bench_runner)
# set_target_properties(bench PROPERTIES EXCLUDE_FROM_ALL TRUE)
# add_custom_command(TARGET bench POST_BUILD COMMAND $<TARGET_FILE:bench_runner>)
