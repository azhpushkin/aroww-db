cmake_minimum_required(VERSION 3.0)


set(DIR_PATH ${CMAKE_BINARY_DIR})


# Source files
set(SOURCES
    common/string_or_tomb.cpp
    common/messages.cpp
    common/serialization.cpp

    engine/engine.cpp
    engine/segment.cpp
    engine/memtable.cpp
    engine/workers.cpp
    network/socket_server.cpp
    network/connection.cpp
    utils/closeable.cpp
)
set(LIB_SOURCES
    common/string_or_tomb.cpp
    common/messages.cpp
    common/serialization.cpp

    lib/aroww.cpp
)

# Also add this paths to include dir for tests
set(TESTABLE_HDR_INCLUDE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Executables and libs
add_executable(aroww-server ${SOURCES} server.cpp)
add_library(aroww-db SHARED ${LIB_SOURCES})
add_library(aroww-db-for-tests SHARED ${SOURCES} ${LIB_SOURCES})

# Target to run server
add_custom_target(serv DEPENDS aroww-server)
set_target_properties(serv PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(
    TARGET serv
    POST_BUILD COMMAND
    $<TARGET_FILE:aroww-server> --datadir ${DIR_PATH}
)
