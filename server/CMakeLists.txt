cmake_minimum_required(VERSION 3.10)
project(server)

# Submoduł nlohmann/json
add_subdirectory(json)

add_executable(server server.cpp)
target_include_directories(server PRIVATE json)

target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)
