cmake_minimum_required(VERSION 3.7)
project(script_engine)
set(CMAKE_CXX_FLAGS "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

file(GLOB SOURCE_FILES
  ./src/*/*.cpp
  ./src/*.cpp
)

include_directories(
  ${SINEN_SOURCE_DIR}/include
  ${SINEN_SOURCE_DIR}/include/sinen
  ${SINEN_SOURCE_DIR}/libs/sol2/include
  ${SINEN_SOURCE_DIR}/libs/Lua/include
)
link_directories(
)
add_library(script_engine STATIC
  ${SOURCE_FILES}
)
target_link_libraries(script_engine
  sinen
)

set_property(TARGET script_engine PROPERTY CXX_STANDARD 23)