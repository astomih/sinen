cmake_minimum_required(VERSION 3.20)
project(script)
set(CMAKE_CXX_FLAGS "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

file(GLOB SOURCE_FILES
        ./src/*/*.cpp
        ./src/*.cpp
)

include_directories(
        ${SINEN_SOURCE_DIR}/include
        ${SINEN_SOURCE_DIR}/include/sinen
        ${SINEN_SOURCE_DIR}/libs/pocketpy/include
)
link_directories(
)
add_library(script STATIC
        ${SOURCE_FILES}
)
target_link_libraries(script
        sinen_core
)

set_property(TARGET script PROPERTY CXX_STANDARD 23)