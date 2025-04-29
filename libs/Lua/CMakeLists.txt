cmake_minimum_required(VERSION 3.10)
project(lua)

file(GLOB_RECURSE LUA_SRC
  include/*.h
  source/*.c
)

add_library(lua STATIC ${LUA_SRC})

target_include_directories(lua PUBLIC ./include)