cmake_minimum_required(VERSION 3.7)
project(sinen)
option(EXTERNAL_ALL_BUILD "Build all external libralies" OFF)
add_subdirectory(./converter)
add_subdirectory(./editor)
add_subdirectory(./test)
add_subdirectory(./usertype)
if(EXTERNAL_ALL_BUILD)
add_subdirectory(./external/SDL2)
add_subdirectory(./external/SDL2_image)
add_subdirectory(./external/SDL2_mixer)
add_subdirectory(./external/SDL2_net)
add_subdirectory(./external/SDL2_ttf)
add_subdirectory(./external/glslang)
set(SDL2_INCLUDE_DIRS
      external/SDL2/include
      external/SDL2_mixer/include
      external/SDL2_image/include
      external/SDL2_ttf/include
      external/SDL2_net/include)
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
      external/ImGui/include
      external/sol2/include
      external/Lua/include
      external/VulkanMemoryAllocator/include
      external/mojoAL/AL
      external/rapidjson/include
      external/mojoAL
)
file(
  GLOB
  SOURCE_FILES
  src/*.cpp
  src/*/*.cpp
  src/render/vulkan/*.cpp
  external/ImGui/source/*.cpp
  external/Lua/source/*.c
  external/mojoAL/*.c)
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
  SDL2_net
  glslang 
  SPIRV 
  glslang-default-resource-limits
)
target_compile_features(sinen PUBLIC cxx_std_20)
