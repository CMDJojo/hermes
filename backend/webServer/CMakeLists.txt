cmake_minimum_required(VERSION 3.16)
project(webServer)

set(CMAKE_CXX_STANDARD 20)

if (WIN32 OR APPLE)
    find_package(Boost REQUIRED COMPONENTS filesystem coroutine regex)
    include_directories(${Boost_INCLUDE_DIR})
endif(WIN32 OR APPLE)

add_library(webServer webServer.cpp webServer.h routeTypes.cpp routeTypes.h sendLambda.h)

if (WIN32 OR APPLE)
    target_link_libraries(webServer PUBLIC ${Boost_LIBRARIES})
    target_include_directories(webServer PUBLIC ${Boost_INCLUDE_DIRS})
endif(WIN32 OR APPLE)

if (UNIX AND NOT APPLE)
    target_link_libraries(webServer pthread)
    target_link_libraries(webServer crypto)
    target_link_libraries(webServer ssl)
    target_link_libraries(webServer boost_regex)
    target_link_libraries(webServer boost_coroutine)
    target_link_libraries(webServer boost_filesystem)
endif (UNIX AND NOT APPLE)
