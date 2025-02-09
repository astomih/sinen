cmake_minimum_required(VERSION 3.7)
project(sinen)
option(EXTERNAL_ALL_BUILD "Build all external libralies" ON)

# Still in development
# add_subdirectory(./editor)

# add_subdirectory(./test)
add_subdirectory(./usertype)
add_subdirectory(./works)

if(EXTERNAL_ALL_BUILD)
  add_subdirectory(./libs/SDL EXCLUDE_FROM_ALL)
  set(DAV1D_ASM OFF)
  add_subdirectory(./libs/SDL_image EXCLUDE_FROM_ALL)
  add_subdirectory(./libs/SDL_mixer EXCLUDE_FROM_ALL)
  add_subdirectory(./libs/SDL_ttf EXCLUDE_FROM_ALL)
  set(SDL3_INCLUDE_DIRS
    libs/SDL/include
    libs/SDL/include/SDL3
    libs/SDL_mixer/include
    libs/SDL_mixer/include/SDL3_mixer
    libs/SDL_image/include
    libs/SDL_image/include/SDL3_image
    libs/SDL_ttf/include
    libs/SDL_ttf/include/SDL3_ttf
  )
else()
  find_package(SDL3 REQUIRED)
endif()

find_package(Vulkan REQUIRED)

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
  sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${SDL3_INCLUDE_DIRS})
target_link_libraries(
  sinen
  SDL3::SDL3
  SDL3_image::SDL3_image
  SDL3_mixer::SDL3_mixer
  SDL3_ttf::SDL3_ttf
)
target_compile_features(sinen PUBLIC cxx_std_20)
