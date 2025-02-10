cmake_minimum_required(VERSION 3.20)
project(sinen)
option(SINEN_EXTERNAL_ALL_BUILD "Build all external libralies" ON)

if(SINEN_EXTERNAL_ALL_BUILD)
  # set(SDL_SHARED OFF)
  # set(SDL_STATIC ON)
  # add_subdirectory(./libs/paranoixa/library/SDL EXCLUDE_FROM_ALL)
  # set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libs/SDL")
endif()

set(PARANOIXA_BUILD_SDL3 ON)
add_subdirectory(./libs/paranoixa EXCLUDE_FROM_ALL)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_BINARY_DIR}/libs/paranoixa/library/SDL")

if(SINEN_EXTERNAL_ALL_BUILD)
  set(DAV1D_ASM OFF)
  add_subdirectory(./libs/SDL_image EXCLUDE_FROM_ALL)
  set(SDL3_INCLUDE_PATHS
    libs/SDL/include
    libs/SDL_image/include
  )
else()
endif()

add_subdirectory(./libs/SDL_mixer EXCLUDE_FROM_ALL)
set(SDLTTF_VENDORED ON)
add_subdirectory(./libs/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(./libs/sol2 EXCLUDE_FROM_ALL)

find_package(Vulkan REQUIRED)

# Still in development
# add_subdirectory(./editor)

# add_subdirectory(./test)
add_subdirectory(./usertype)
add_subdirectory(./works)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif()

set(SINEN_INCLUDE_DIRS
  .
  ./include
  ./include/sinen
  ./usertype/include
  libs/imgui
  libs/imgui/backends
  libs/sol2/include
  libs/Lua/include
  libs/VulkanMemoryAllocator/include
  libs/mojoAL-SDL3/AL
  libs/rapidjson/include
  libs/mojoAL-SDL3
  libs/tinygltf
  ${SDL3_INCLUDE_FILES}
  libs/SDL_mixer/include
  libs/SDL_ttf/include
  libs/paranoixa/include
)
file(
  GLOB
  SOURCE_FILES
  src/*.cpp
  src/*/*.cpp
  src/render/vulkan/*.cpp
  libs/imgui/*.cpp
  libs/imgui/backends/imgui_impl_sdl3.cpp
  libs/imgui/backends/imgui_impl_vulkan.cpp
  libs/Lua/source/*.c
  libs/mojoAL-SDL3/*.c)
file(
  GLOB_RECURSE
  HEADER_FILES
  include/*.hpp
  src/*.hpp
  include/*.inl
)
add_library(sinen STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(
  sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${SDL3_INCLUDE_PATHS})

if(MSVC)
  target_link_libraries(
    sinen
    paranoixa
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
  )
else()
  target_link_libraries(
    sinen
    paranoixa
    SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
  )
endif()

target_compile_features(sinen PUBLIC cxx_std_20)
