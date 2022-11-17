cmake_minimum_required(VERSION 3.20)
project(backend)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(webServer)

add_executable(backend main.cpp)
add_executable(server server.cpp)

target_link_libraries(server webServer)
