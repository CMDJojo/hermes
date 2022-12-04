cmake_minimum_required(VERSION 3.20)
project(backend)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(webServer)

find_package(Boost REQUIRED COMPONENTS filesystem coroutine json)
include_directories(${Boost_INCLUDE_DIR})

add_executable(backend main.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp
        routingCacher.cpp routingCacher.h
        gauss-kruger/gausskruger.cpp gauss-kruger/gausskruger.h
        people.h people.cpp
        boardingStatistics.cpp boardingStatistics.h
        endToEndEvaluator.cpp endToEndEvaluator.h
        binarySearch.cpp binarySearch.h
        prox.cpp prox.h)

add_executable(server server.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp
        people.h people.cpp
        gauss-kruger/gausskruger.cpp gauss-kruger/gausskruger.h
        routingCacher.cpp routingCacher.h
        boardingStatistics.cpp boardingStatistics.h
        endToEndEvaluator.cpp endToEndEvaluator.h
        binarySearch.cpp binarySearch.h
        prox.cpp prox.h
        lineRegister.cpp lineRegister.h)

add_executable(test test.cpp
        csvLoader.h csvLoaderTypes.h
        gtfsTypes.h gtfsTypes.cpp
        routing.h routing.cpp
        routingCacher.cpp routingCacher.h
        people.h people.cpp
        gauss-kruger/gausskruger.cpp gauss-kruger/gausskruger.h
        boardingStatistics.cpp boardingStatistics.h
        binarySearch.cpp binarySearch.h
        endToEndEvaluator.cpp endToEndEvaluator.h
        prox.cpp prox.h)

target_link_libraries(server webServer)

file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data SYMBOLIC)
