cmake_minimum_required(VERSION 3.20)
project(frontend)

add_custom_target (
        frontend
        COMMAND npm run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dist/ DESTINATION ${CMAKE_BINARY_DIR}/backend/public)
