cmake_minimum_required(VERSION 3.20)
project(rt_shader_compiler)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

file(GLOB_RECURSE HEADER_FILES include/*.hpp)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
include_directories(include)
add_library(rt_shader_compiler STATIC ${SOURCE_FILES} ${HEADER_FILES})