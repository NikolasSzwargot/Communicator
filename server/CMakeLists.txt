cmake_minimum_required(VERSION 3.10)
project(Communicator)

# Submoduł nlohmann/json
add_subdirectory(json)

add_executable(Communicator server.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

target_include_directories(Communicator PRIVATE json)

target_link_libraries(Communicator PRIVATE nlohmann_json::nlohmann_json)
