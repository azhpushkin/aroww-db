cmake_minimum_required(VERSION 3.0)

# Source files
set(COMMON_SRCS engine/engine.cpp network/socket_server.cpp)
set(SERVER_SRCS ${COMMON_SRCS} server.cpp)
set(LIBRARY_SRCS lib/aroww.cpp)
set(CLIENT_SRCS client.cpp ${LIBRARY_SRCS})

# Protobuf generation and configuration
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS commands.proto)

# Set files to be import by tests
set(__TESTABLE ${PROTO_SRCS})
foreach(SRC ${COMMON_SRCS})
    get_filename_component(SRC_ABS ${SRC} ABSOLUTE) 
    LIST(APPEND __TESTABLE ${SRC_ABS})
ENDFOREACH(SRC)
set(TESTABLE_SRC ${__TESTABLE} PARENT_SCOPE)
set(TESTABLE_HDR ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)


# Include to use protobufs
include_directories(PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# Link libraries
link_libraries(${Protobuf_LITE_LIBRARIES})
link_libraries(fmt::fmt)

# Server executable
add_executable(aroww-server ${SERVER_SRCS} ${PROTO_SRCS})
add_executable(aroww-client ${CLIENT_SRCS} ${PROTO_SRCS})
add_library(aroww-db SHARED ${LIBRARY_SRCS} ${PROTO_SRCS})

