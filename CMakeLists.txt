cmake_minimum_required(VERSION 3.7)
project(sinen)
add_subdirectory(./script)
set(SINEN_INCLUDE_DIRS
      .
      ./include
      ./include/sinen
      ./script/include
      external/ImGui/include
      external/sol2/include
      external/Lua/include
      external/VulkanMemoryAllocator/include
      external/mojoAL/AL
      external/rapidjson/include
      external/mojoAL
)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
add_subdirectory(./tools/model_converter)
add_subdirectory(./editor)
add_subdirectory(./game)
add_subdirectory(./test)
option(ALL_BUILD "Build all targets" OFF)
if(ALL_BUILD)
add_subdirectory(./external/SDL2)
add_subdirectory(./external/SDL2_image)
add_subdirectory(./external/SDL2_mixer)
add_subdirectory(./external/SDL2_net)
add_subdirectory(./external/SDL2_ttf)
set(SDL2_INCLUDE_DIRS
      external/SDL2/include
      external/SDL2_mixer/include
      external/SDL2_image/include
      external/SDL2_ttf/include
      external/SDL2_net/include)
else()
find_package(SDL2 REQUIRED)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
find_package(Vulkan REQUIRED)
file(
  GLOB
  SOURCE_FILES
  src/*/*.cpp
  src/render/vulkan/*.cpp
  external/ImGui/source/*.cpp
  external/Lua/source/*.c
  external/mojoAL/*.c)
add_library(sinen STATIC ${SOURCE_FILES})
target_compile_features(sinen PUBLIC cxx_std_20)
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
)
