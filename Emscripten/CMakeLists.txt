cmake_minimum_required(VERSION 3.7)
project(nen)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example/bin)
SET(CMAKE_BUILD_TYPE_INIT "Release")
set(CMAKE_C_COMPILER "emcc")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./build)

file(GLOB SOURCE_FILES 
	../src/Nen/*/*.cpp
	../src/Nen/Render/OpenGLES/*.cpp
	../libs/ImGui/source/imgui.cpp
	../libs/ImGui/source/imgui_draw.cpp
	../libs/ImGui/source/imgui_demo.cpp
	../libs/ImGui/source/imgui_impl_opengl3.cpp
	../libs/ImGui/source/imgui_impl_sdl.cpp
	../libs/ImGui/source/imgui_widgets.cpp
	../libs/tinyobjloader/tiny_ob_loader.cc
)
add_library(nen STATIC ${SOURCE_FILES})
set(NEN_INCLUDE_DIRECTORIES 
	.
	../include/Nen
	../libs/tinyobjloader 
	../libs/Effekseer/include/Effekseer
	../libs/Effekseer/include/EffekseerRendererGL
	../libs/Effekseer/include
	../libs/ImGui/include
	../libs/sol2/include
	../libs/Lua/include
	../libs/rapidjson/include
)


target_include_directories(nen PUBLIC
	${NEN_INCLUDE_DIRECTORIES}
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s USE_SDL_NET=2 -std=c++2b")
