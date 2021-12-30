cmake_minimum_required(VERSION 3.7)
project(nen)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS "-std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG -march=native")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./build)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./build)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)


file(GLOB SOURCE_FILES 
	src/Nen/*/*.cpp
	src/Nen/Render/Vulkan/*.cpp
	src/Nen/Render/OpenGL/*.cpp
	libs/ImGui/source/*.cpp
	libs/Lua/source/*.c
)


add_library(nen STATIC 
	${SOURCE_FILES} 
	)

set(NEN_INCLUDE_DIRECTORIES 
	.
	include
	include/Nen
	libs/ImGui/include
	libs/sol2/include
	libs/Lua/include
	libs/rapidjson/include
)


target_include_directories(nen PUBLIC
	${NEN_INCLUDE_DIRECTORIES}
	${Vulkan_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}/
	${OpenAL_INCLUDE_DIRS}
)
