cmake_minimum_required(VERSION 3.7)
project(nen)
if(ANDROID)
	set(CMAKE_CXX_FLAGS "-std=c++20 -DANDROID -Wno-implicit-function-declaration")
	find_library(SDL2 SDL2)
	find_library(SDL2_image SDL2_image)
	find_library(SDL2_mixer SDL2_mixer)
	find_library(SDL2_ttf SDL2_ttf)
	find_library(SDL2_net SDL2_net)
	find_library(GLESv3 GLESv3)
	find_library(log-lib log)
	file(GLOB SOURCE_FILES
			src/Nen/*/*.cpp
			src/Nen/Render/OpenGLES/*.cpp
			libs/ImGui/source/gles/*.cpp
			libs/mojoAL/mojoal.c
			libs/Lua/source/*.c
			)
	add_library(nen SHARED
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
			libs/mojoAL
			../SDL2/include
			../SDL2_net
			../SDL2_image
			../SDL2_mixer/include
			../SDL2_ttf
			)
	target_link_libraries(nen PRIVATE
			SDL2
			SDL2_image
			SDL2_mixer
			SDL2_ttf
			SDL2_net
			GLESv3
			log
			)

	target_include_directories(nen PUBLIC
			${NEN_INCLUDE_DIRECTORIES}
			)
	else()
set(CMAKE_CXX_FLAGS "-std=c++2a -Wno-subobject-linkage")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG -march=native")
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
)


target_include_directories(nen PUBLIC
	${NEN_INCLUDE_DIRECTORIES}
	${Vulkan_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}/
	${OpenAL_INCLUDE_DIRS}
)
endif()
