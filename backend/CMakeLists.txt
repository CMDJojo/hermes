cmake_minimum_required(VERSION 3.20)
project(backend)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(webServer)

find_package(Boost REQUIRED COMPONENTS filesystem coroutine json)
include_directories(${Boost_INCLUDE_DIR})

add_executable(backend main.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp)
add_executable(server server.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp)
add_executable(test test.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp)

add_executable(person
        csvLoader.h
        gtfsTypes.h
        personType.h person.cpp
        gauss-kruger/gausskruger.cpp gauss-kruger/gausskruger.h)

target_link_libraries(server webServer)

file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data SYMBOLIC)
