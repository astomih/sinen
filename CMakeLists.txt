cmake_minimum_required(VERSION 3.7)
project(nen)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist)
SET(CMAKE_BUILD_TYPE_INIT "Release")

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(CMAKE_C_COMPILER "emcc")
endif ()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin)

file(GLOB SOURCE_FILES 
	src/nen/*.cpp
	src/nen/OpenGL/ShaderGL.cpp
	src/nen/ES/*.cpp
	src/nen/component/*.cpp
	src/nen/actor/*.cpp
	src/nen/scene/*.cpp
	lib/imgui/source/imgui.cpp
	lib/imgui/source/imgui_draw.cpp
	lib/imgui/source/imgui_demo.cpp
	lib/imgui/source/imgui_impl_opengl3.cpp
	lib/imgui/source/imgui_impl_sdl.cpp
	lib/imgui/source/imgui_widgets.cpp
	)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	#TODO
endif ()

include_directories(
	.
	./include/nen 
	./lib/tinyobjloader 
	./lib/Effekseer/include/Effekseer
	./lib/Effekseer/include/EffekseerRendererGL
	./lib/Effekseer/include
	./lib/imgui/include
	./lib/sol2-3.2.2/include
	./lib/lua
)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	#TODO
endif ()

add_library(nen STATIC ${SOURCE_FILES})
target_link_libraries(nen ./lib/lua/lua)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -O2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s USE_SDL_NET=2 -std=c++2b")

else ()
	#TODO
endif ()
