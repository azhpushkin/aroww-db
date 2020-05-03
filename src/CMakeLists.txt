cmake_minimum_required(VERSION 3.0)

# set(CMAKE_CXX_CLANG_TIDY 
#   clang-tidy;
#   -checks=*;)

set(DIR_PATH ${CMAKE_BINARY_DIR})


# Protobuf generation and configuration
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS commands.proto)

# Source files
set(SERV_ONLY
    engine/append_log.cpp
    network/socket_server.cpp
    network/connection.cpp
)
set(SOURCES
    lib/aroww.cpp
    utils/client_utils.cpp
    ${PROTO_SRCS}
)
# Separate args to lower final size of library
set(LIB_SOURCES lib/aroww.cpp ${PROTO_SRCS})


# Include to use protobufs
include_directories(PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# Also add this paths to include dir for tests
set(TESTABLE_HDR_INCLUDE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Executables and libs
add_executable(aroww-server ${SOURCES} ${SERV_ONLY} server.cpp)
add_executable(aroww-client ${SOURCES} client.cpp)
add_library(aroww-db SHARED ${LIB_SOURCES})
add_library(aroww-db-for-tests SHARED ${SOURCES} ${SERV_ONLY})

# Target to run server
add_custom_target(serv DEPENDS aroww-server)
set_target_properties(serv PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(
    TARGET serv
    POST_BUILD COMMAND
    $<TARGET_FILE:aroww-server> --datadir ${DIR_PATH}
)

# Target to run client
add_custom_target(cli DEPENDS aroww-client)
set_target_properties(cli PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(TARGET cli POST_BUILD COMMAND $<TARGET_FILE:aroww-client>)