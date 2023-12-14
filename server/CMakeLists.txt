cmake_minimum_required(VERSION 3.10)
project(asd)

# Submoduł nlohmann/json
add_subdirectory(json)

add_executable(asd server.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

target_include_directories(asd PRIVATE json)

target_link_libraries(asd PRIVATE nlohmann_json::nlohmann_json)
