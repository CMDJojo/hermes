cmake_minimum_required(VERSION 3.20)
project(backend)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(webServer)

add_executable(backend main.cpp csvLoader.h gtfsTypes.h routing.cpp routing.h)
add_executable(server server.cpp)

target_link_libraries(server webServer)

file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data SYMBOLIC)