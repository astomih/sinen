cmake_minimum_required(VERSION 3.7)
project(sinen)
option(EXTERNAL_ALL_BUILD "Build all external libralies" OFF)
add_subdirectory(./editor)
add_subdirectory(./test)
add_subdirectory(./usertype)
add_subdirectory(./works)
if(EXTERNAL_ALL_BUILD)
add_subdirectory(./libs/SDL)
add_subdirectory(./libs/SDL_image)
add_subdirectory(./libs/SDL_mixer)
add_subdirectory(./libs/SDL_ttf)
add_subdirectory(./libs/glslang)
set(SDL2_INCLUDE_DIRS
      libs/SDL/include
      libs/SDL_mixer/include
      libs/SDL_image/include
      libs/SDL_ttf/include)
else()
find_package(SDL2 REQUIRED)
endif()
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(SINEN_INCLUDE_DIRS
      .
      ./include
      ./include/sinen
      ./usertype/include
      libs/ImGui/include
      libs/sol2/include
      libs/Lua/include
      libs/VulkanMemoryAllocator/include
      libs/mojoAL/AL
      libs/rapidjson/include
      libs/mojoAL
      libs/tinygltf
)
file(
  GLOB
  SOURCE_FILES
  src/*.cpp
  src/*/*.cpp
  src/render/vulkan/*.cpp
  libs/ImGui/source/*.cpp
  libs/Lua/source/*.c
  libs/mojoAL/*.c)
add_library(sinen STATIC ${SOURCE_FILES})
target_include_directories(
  sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
target_link_libraries(
  sinen
  SDL2
  SDL2main
  SDL2_image
  SDL2_mixer
  SDL2_ttf
  glslang 
  SPIRV 
  glslang-default-resource-limits
)
target_compile_features(sinen PUBLIC cxx_std_20)
