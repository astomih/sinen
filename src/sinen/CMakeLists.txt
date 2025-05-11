cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen_core)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif()

set(SINEN_INCLUDE_DIRS
  ${SINEN_SOURCE_DIR}
  ${SINEN_SOURCE_DIR}/include
  ${SINEN_SOURCE_DIR}/include/sinen
  ${SINEN_SOURCE_DIR}/libs/paranoixa/library/imgui
  ${SINEN_SOURCE_DIR}/libs/paranoixa/library/imgui/backends
  ${SINEN_SOURCE_DIR}/libs/sol2/include
  ${SINEN_SOURCE_DIR}/libs/Lua/include
  ${SINEN_SOURCE_DIR}/libs/pocketpy/include
  ${SINEN_SOURCE_DIR}/libs/mojoAL-SDL3/AL
  ${SINEN_SOURCE_DIR}/libs/rapidjson/include
  ${SINEN_SOURCE_DIR}/libs/mojoAL-SDL3
  ${SINEN_SOURCE_DIR}/libs/SDL/include
  ${SINEN_SOURCE_DIR}/libs/SDL_image/include
  ${SINEN_SOURCE_DIR}/libs/tinygltf
  ${SINEN_SOURCE_DIR}/libs/SDL_mixer/include
  ${SINEN_SOURCE_DIR}/libs/SDL_ttf/include
  ${SINEN_SOURCE_DIR}/libs/paranoixa/include
  ${SINEN_SOURCE_DIR}/libs/paranoixa/source/imgui_backend
  ${SINEN_SOURCE_DIR}/src/usertype/include
  ${SINEN_SOURCE_DIR}/src/rt_shader_compiler/include
  ${SINEN_SOURCE_DIR}/libs/assimp/include
  ${SINEN_SOURCE_DIR}/libs/glm
  ${SINEN_SOURCE_DIR}/libs/SDL/include
  ${SINEN_SOURCE_DIR}/libs/SDL_image/include
)
file(
  GLOB_RECURSE
  SOURCE_FILES
  ${SINEN_SOURCE_DIR}/src/sinen/*.cpp
  ${SINEN_SOURCE_DIR}/libs/mojoAL-SDL3/*.c
)
file(
  GLOB_RECURSE
  HEADER_FILES
  ${SINEN_SOURCE_DIR}/include/*.hpp
  ${SINEN_SOURCE_DIR}/src/*.hpp
  ${SINEN_SOURCE_DIR}/include/*.inl
)
add_library(sinen_core STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(
  sinen_core PUBLIC ${SINEN_INCLUDE_DIRS})

if(NOT SINEN_EXTERNAL_ALL_BUILD)
  target_link_libraries(
    sinen_core
    paranoixa
    paranoixa_imgui_backend
    SDL3::SDL3
    SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf
    rt_shader_compiler
    assimp
    lua
  )
else()
  target_link_libraries(
    sinen_core
    paranoixa
    paranoixa_imgui_backend
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    rt_shader_compiler
    assimp
    lua
    pocketpy
  )
endif()

set_property(TARGET sinen_core PROPERTY CXX_STANDARD 23)