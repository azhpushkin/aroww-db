cmake_minimum_required(VERSION 3.0)

# Add TESTABLE_HDR and TESTABLE_SRC to use files from src/
include_directories(PUBLIC ${TESTABLE_HDR})
set(TEST_FILES tests_engine.cpp tests_main.cpp)

# Catch2 test runner compile
find_package(Catch2 REQUIRED)
add_executable(test_runner ${TEST_FILES} ${TESTABLE_SRC} ${PROTO_SRCS})
set_target_properties(test_runner PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(test_runner Catch2::Catch2)

# Custom targets to run tests, always rebuild
set(TEST_TARGETS_NAMES test tests catch catch2)

foreach(TARGET_NAME ${TEST_TARGETS_NAMES})
    add_custom_target(${TARGET_NAME} DEPENDS test_runner)
    set_target_properties(${TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND $<TARGET_FILE:test_runner>)
endforeach(TARGET_NAME)
