cmake_minimum_required(VERSION 3.0)

# set(CMAKE_CXX_CLANG_TIDY 
#   clang-tidy;
#   -checks=*;)

set(DIR_PATH ${CMAKE_BINARY_DIR})


# Source files
set(SOURCES
    engine/engine.cpp
    engine/segment.cpp
    network/socket_server.cpp
    network/connection.cpp
)
# Separate args to lower final size of library
set(LIB_SOURCES lib/aroww.cpp)

# Also add this paths to include dir for tests
set(TESTABLE_HDR_INCLUDE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

# Executables and libs
add_executable(aroww-server ${SOURCES} server.cpp)
add_library(aroww-db SHARED ${LIB_SOURCES})
add_library(aroww-db-for-tests SHARED ${SOURCES})

# Target to run server
add_custom_target(serv DEPENDS aroww-server)
set_target_properties(serv PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_custom_command(
    TARGET serv
    POST_BUILD COMMAND
    $<TARGET_FILE:aroww-server> --datadir ${DIR_PATH}
)
