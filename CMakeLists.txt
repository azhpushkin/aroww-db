cmake_minimum_required(VERSION 3.0)
project(aroww-db)
include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/src)


# CXX compiler settings
set(CMAKE_CXX_COMPILER /bin/clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wpedantic")

# threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Connect libs for all targets
find_package(spdlog 1 REQUIRED)
find_package(fmt 6 REQUIRED)
find_package(Protobuf REQUIRED)
set(Protobuf_USE_STATIC_LIBS ON)

link_libraries(${Protobuf_LITE_LIBRARIES})
link_libraries(fmt::fmt)

add_subdirectory(src)
add_subdirectory(tests)
