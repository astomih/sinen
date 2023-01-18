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
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
if(ANDROID)
  set(CMAKE_CXX_FLAGS "-std=c++20 -DANDROID")
  file(
    GLOB_RECURSE
    SOURCE_FILES
    src/*/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/gles/*.cpp
    external/mojoAL/mojoal.c
    external/Lua/source/*.c)
  add_library(sinen SHARED ${SOURCE_FILES})
  target_include_directories(sinen PUBLIC ${SINEN_INCLUDE_DIRS})
  target_link_libraries(
    sinen
    SDL2
    SDL2_image
    SDL2_mixer
    SDL2_ttf
    SDL2_net
    GLESv3
    log)
elseif(EMSCRIPTEN)
  option(BUILD_DEMO "Build Demo project" ON)
  if(BUILD_DEMO)
    add_subdirectory(./demo)
  endif()
  file(
    GLOB
    SOURCE_FILES
    src/*/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/gles/*.cpp
    external/Lua/source/*.c)
  add_library(sinen STATIC ${SOURCE_FILES})
  target_include_directories(sinen PUBLIC ${SINEN_INCLUDE_DIRS})
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s USE_WEBGL2=1 \
		-s USE_SDL=2 \
		-s USE_SDL_IMAGE=2 \
		-s USE_SDL_TTF=2 \
		-s USE_SDL_MIXER=2 \
		-s USE_SDL_NET=2 \
		-std=c++20")
  if(CMAKE_HOST_WIN32)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-s SDL2_IMAGE_FORMATS="[""png""]" \
    ")
  else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-s SDL2_IMAGE_FORMATS='[\"png\"]' \
		")
  endif()
else()
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
  else()
  find_package(SDL2 REQUIRED)
  endif()
  set(CMAKE_EXPORT_COMPILE_COMMANDS true)
  find_package(Vulkan REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
  file(
    GLOB
    SOURCE_FILES
    src/*/*.cpp
    src/render/vulkan/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/*.cpp
    external/Lua/source/*.c
    external/mojoAL/*.c)
  add_library(sinen STATIC ${SOURCE_FILES})
  target_compile_features(sinen PUBLIC cxx_std_20)
  target_include_directories(
    sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS}
                 ${SDL2_INCLUDE_DIRS})
  target_link_libraries(
    sinen
    SDL2
    SDL2main
    SDL2_image
    SDL2_mixer
    SDL2_ttf
    SDL2_net
  )
endif()
