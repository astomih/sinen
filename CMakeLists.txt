cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)
option(SINEN_EXTERNAL_ALL_BUILD "Build all external libralies" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})

if(SINEN_EXTERNAL_ALL_BUILD)
  set(SDL_SHARED ${BUILD_SHARED_LIBS})
  set(SDL_STATIC NOT ${BUILD_SHARED_LIBS})
  add_subdirectory(./libs/SDL EXCLUDE_FROM_ALL)
endif()

if(SINEN_EXTERNAL_ALL_BUILD)
  set(SDLIMAGE_AVIF OFF)
  set(SDLIMAGE_JXL OFF)
  set(SDLIMAGE_WEBP OFF)
  set(SDLIMAGE_TIF OFF)
  set(SDLIMAGE_LBM OFF)
  set(SDLIMAGE_PCX OFF)
  set(SDLIMAGE_PNM OFF)
  set(SDLIMAGE_XCF OFF)
  set(SDLIMAGE_XPM OFF)
  set(SDLIMAGE_XV OFF)
  set(SDLIMAGE_QOI OFF)
  set(SDLIMAGE_TGA OFF)
  set(SDLIMAGE_VENDORED ON)

  set(DAV1D_ASM OFF)
  add_subdirectory(./libs/SDL_image EXCLUDE_FROM_ALL)
  set(SDL3_INCLUDE_PATHS
    libs/SDL/include
    libs/SDL_image/include
  )
  set(SDLTTF_VENDORED ON)
  set(SDLTTF_PLUTOSVG OFF)
  add_subdirectory(./libs/SDL_ttf EXCLUDE_FROM_ALL)
  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
  add_subdirectory(./libs/assimp EXCLUDE_FROM_ALL)
else()
  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

add_subdirectory(./libs/SDL_mixer EXCLUDE_FROM_ALL)
add_subdirectory(./libs/Lua)
add_subdirectory(./libs/sol2 EXCLUDE_FROM_ALL)

add_subdirectory(./src/sinen)

set(PARANOIXA_BUILD_SDL3 OFF)
add_subdirectory(./libs/paranoixa EXCLUDE_FROM_ALL)

# Still in development
add_subdirectory(./src/editor)

# add_subdirectory(./test)
add_subdirectory(./src/usertype EXCLUDE_FROM_ALL)
add_subdirectory(./src/rt_shader_compiler EXCLUDE_FROM_ALL)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif()

add_executable(sinen src/main.cpp docs/icon/icon.rc)
target_include_directories(
  sinen PUBLIC ${SINEN_CORE_INCLUDE_DIRS} ${SDL3_INCLUDE_PATHS})

if(NOT SINEN_EXTERNAL_ALL_BUILD)
  target_link_libraries(
    sinen
    PUBLIC
    sinen_core
    script_engine
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
    sinen
    PUBLIC
    sinen_core
    script_engine
    paranoixa
    paranoixa_imgui_backend
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    rt_shader_compiler
    assimp
    lua
  )
endif()

set_property(TARGET sinen PROPERTY CXX_STANDARD 23)