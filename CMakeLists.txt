cmake_minimum_required(VERSION 3.7)
project(nen)
if(ANDROID)
	set(CMAKE_CXX_FLAGS "-std=c++20 -DANDROID")

	file(GLOB_RECURSE SOURCE_FILES
			src/Nen/*/*.cpp
			src/Nen/Render/OpenGL/*.cpp
			libs/ImGui/source/gles/*.cpp
			libs/mojoAL/mojoal.c
			libs/Lua/source/*.c
			)
	add_library(nen STATIC ${SOURCE_FILES})
	set(INCLUDE_DIRS
			./include
			./include/Nen
			libs/ImGui/include
			libs/sol2/include
			libs/Lua/include
			libs/rapidjson/include
			libs/mojoAL
			../SDL2/include
			../SDL2_net
			../SDL2_image
			../SDL2_mixer/include
			../SDL2_ttf
			)
	target_include_directories(nen PUBLIC ${INCLUDE_DIRS})
	target_link_libraries(nen SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL2_net GLESv3 log)
elseif(EMSCRIPTEN)

	file(GLOB SOURCE_FILES 
		src/Nen/*/*.cpp
		src/Nen/Engine/Engine.cpp
		src/Nen/Render/OpenGL/*.cpp
		libs/ImGui/source/gles/*.cpp
	)
	add_library(nen STATIC ${SOURCE_FILES})
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
	)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 \
		-Wno-unused-command-line-argument \
		-s USE_WEBGL2=1 \
		-s FULL_ES3=1 \
		-s SDL2_IMAGE_FORMATS='[\"png\"]' \
		-s USE_SDL=2 \
		-s USE_SDL_IMAGE=2 \
		-s USE_SDL_TTF=2 \
		-s USE_SDL_MIXER=2 \
		-s USE_SDL_NET=2 \
		-s WASM=1 \
		-std=c++20"
	)
else()	
	if(WIN32)
	set(CMAKE_CXX_STANDARD 20)
	else()
	set(CMAKE_CXX_FLAGS "-std=c++2a")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -g -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s -DNDEBUG -march=native")
	set(CMAKE_EXPORT_COMPILE_COMMANDS true)
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
			./include
			./include/Nen
			libs/ImGui/include
			libs/sol2/include
			libs/Lua/include
			libs/rapidjson/include
			libs/VulkanMemoryAllocator/include
			)


	target_include_directories(nen PUBLIC
			${NEN_INCLUDE_DIRECTORIES}
			${Vulkan_INCLUDE_DIRS}
			${SDL2_INCLUDE_DIRS}
			${OpenAL_INCLUDE_DIRS}
			)
endif()
