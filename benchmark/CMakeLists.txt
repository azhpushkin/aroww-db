cmake_minimum_required(VERSION 3.0)

include_directories(PUBLIC ${PROJECT_SOURCE_DIR}/include)


find_package(PostgreSQL REQUIRED)
add_executable(bench ar.cpp run.cpp pg.cpp)
add_dependencies(bench aroww-db)
target_link_libraries(bench ${PostgreSQL_LIBRARIES})
target_link_libraries(bench aroww-db)

# Do not run by default
set_target_properties(bench PROPERTIES EXCLUDE_FROM_ALL TRUE)
