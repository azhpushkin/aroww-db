cmake_minimum_required(VERSION 3.0)

# Source files
set(COMMON_SRCS engine/engine.cpp network/socket_server.cpp)
set(SERVER_SRCS ${COMMON_SRCS} server.cpp)
set(LIBRARY_SRCS lib/aroww.cpp)
set(CLIENT_SRCS client.cpp utils/client_utils.cpp ${LIBRARY_SRCS})

# Protobuf generation and configuration
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS commands.proto)

# Set files to be import by tests
set(__TESTABLE ${PROTO_SRCS})
foreach(SRC ${COMMON_SRCS} ${LIBRARY_SRCS} utils/client_utils.cpp)
    get_filename_component(SRC_ABS ${SRC} ABSOLUTE) 
    LIST(APPEND __TESTABLE ${SRC_ABS})
endforeach(SRC)
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


add_custom_target(serv DEPENDS aroww-server)
set_target_properties(serv PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(TARGET serv POST_BUILD COMMAND $<TARGET_FILE:aroww-server>)


add_custom_target(cli DEPENDS aroww-client)
set_target_properties(cli PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(TARGET cli POST_BUILD COMMAND $<TARGET_FILE:aroww-client>)