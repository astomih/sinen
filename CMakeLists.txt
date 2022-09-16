cmake_minimum_required(VERSION 3.7)
project(sinen)
if(ANDROID)
  set(CMAKE_CXX_FLAGS "-std=c++20 -DANDROID")
  file(
    GLOB_RECURSE
    SOURCE_FILES
    src/*/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/gles/*.cpp
    external/ImGuiColorTextEdit/*.cpp
    external/mojoAL/mojoal.c
    external/Lua/source/*.c)
  add_library(sinen STATIC ${SOURCE_FILES})
  set(INCLUDE_DIRS
      ./include
      ./include/sinen
      external/ImGui/include
      external/ImGuiColorTextEdit
      external/imgui_markdown
      external/sol2/include
      external/Lua/include
      external/mojoAL
      ../SDL2/include
      ../SDL2_net
      ../SDL2_image
      ../SDL2_mixer/include
      ../SDL2_ttf)
  target_include_directories(sinen PUBLIC ${INCLUDE_DIRS})
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
  file(
    GLOB
    SOURCE_FILES
    src/*/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/gles/*.cpp
    external/ImGuiColorTextEdit/*.cpp
    external/Lua/source/*.c)
  add_library(sinen STATIC ${SOURCE_FILES})
  set(SINEN_INCLUDE_DIRECTORIES
      .
      ./include
      ./include/sinen
      external/ImGui/include
      external/ImGuiColorTextEdit
      external/imgui_markdown
      external/sol2/include
      external/Lua/include)
  target_include_directories(sinen PUBLIC ${SINEN_INCLUDE_DIRECTORIES})
  if(WIN32)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s USE_WEBGL2=1 \
		-s SDL2_IMAGE_FORMATS="[""png""]" \
		-s USE_SDL=2 \
		-s USE_SDL_IMAGE=2 \
		-s USE_SDL_TTF=2 \
		-s USE_SDL_MIXER=2 \
		-s USE_SDL_NET=2 \
		-std=c++20")
  else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s USE_WEBGL2=1 \
		-s SDL2_IMAGE_FORMATS='[\"png\"]' \
		-s USE_SDL=2 \
		-s USE_SDL_IMAGE=2 \
		-s USE_SDL_TTF=2 \
		-s USE_SDL_MIXER=2 \
		-s USE_SDL_NET=2 \
		-std=c++20")
  endif()
else()
  add_subdirectory(./tools/model_converter)
  add_subdirectory(./editor)
  set(CMAKE_EXPORT_COMPILE_COMMANDS true)
  find_package(Vulkan REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(SDL2 REQUIRED)
  file(
    GLOB
    SOURCE_FILES
    src/*/*.cpp
    src/render/vulkan/*.cpp
    src/render/opengl/*.cpp
    external/ImGui/source/*.cpp
    external/ImGuiColorTextEdit/*.cpp
    external/Lua/source/*.c
    external/mojoAL/*.c)
  add_library(sinen STATIC ${SOURCE_FILES})
  target_compile_features(sinen PUBLIC cxx_std_20)
  set(SINEN_INCLUDE_DIRECTORIES
      .
      ./include
      ./include/sinen
      external/ImGui/include
      external/ImGuiColorTextEdit
      external/imgui_markdown
      external/sol2/include
      external/Lua/include
      external/VulkanMemoryAllocator/include
      external/mojoAL/AL
      external/mojoAL)
  target_include_directories(
    sinen PUBLIC ${SINEN_INCLUDE_DIRECTORIES} ${Vulkan_INCLUDE_DIRS}
                 ${SDL2_INCLUDE_DIRS})
endif()
