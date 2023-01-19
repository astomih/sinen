cmake_minimum_required(VERSION 3.7)
project(script_engine)
set(CMAKE_CXX_FLAGS "-std=c++2a -O2 -DNDEBUG")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

file(GLOB SOURCE_FILES
	./src/*/*.cpp 
	./src/*.cpp
  ../external/Lua/source/*.c
)

include_directories(
  ../include
  ../include/sinen
  ../src
  ../external/sol2/include
  ../external/Lua/include
)
link_directories(
)
add_library(script_engine STATIC
	${SOURCE_FILES}
)
target_link_libraries(script_engine
  sinen 
)

