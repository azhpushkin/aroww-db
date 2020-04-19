cmake_minimum_required(VERSION 3.0)
project(aroww-db)
message(${PROJECT_SOURCE_DIR}/src)
include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/src)


# CXX compiler settings
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wpedantic")

# Source files
set(SOURCES src/engine/engine.cpp src/network/socket_server.cpp)
set(SERVER_SRC src/server.cpp)

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# 3rd-party libraries
find_package(spdlog 1 REQUIRED)
find_package(fmt 6 REQUIRED)

# Protobuf generation and configuration
find_package(Protobuf REQUIRED)
set(Protobuf_USE_STATIC_LIBS ON)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/commands.proto)

# Include to use protobufs
include_directories(PUBLIC ${CMAKE_BINARY_DIR})
# Link libraries
link_libraries(${Protobuf_LITE_LIBRARIES})
link_libraries(fmt::fmt)

# Server executable
add_executable(${PROJECT_NAME} ${SOURCES} ${SERVER_SRC} ${PROTO_SRCS})
add_executable(client src/lib/aroww.cpp src/client.cpp ${PROTO_SRCS})
add_library(arowwdb SHARED src/lib/aroww.cpp ${PROTO_SRCS})

# Catch2 tests
find_package(Catch2 REQUIRED)
set(TEST_FILES tests/tests_engine.cpp tests/tests_main.cpp)
add_executable(test ${TEST_FILES} ${SOURCES} ${PROTO_SRCS})
target_link_libraries(test Catch2::Catch2)
