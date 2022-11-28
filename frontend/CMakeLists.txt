cmake_minimum_required(VERSION 3.20)
project(frontend)

add_custom_target (
        frontend
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET frontend POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/dist ${CMAKE_BINARY_DIR}/backend/public)