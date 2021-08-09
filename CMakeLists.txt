cmake_minimum_required(VERSION 3.7)
project(nen)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist)
SET(CMAKE_BUILD_TYPE_INIT "Release")

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(CMAKE_C_COMPILER "emcc")
endif ()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin)

set(SOURCE_FILES 
	Engine/source/AudioEvent.cpp 
	Engine/source/AudioSystem.cpp 
	Engine/source/DebugLog.cpp 
	Engine/source/Effect.cpp 
	Engine/source/Font.cpp 
	Engine/source/InputSystem.cpp 
	Engine/source/Math.cpp 
	Engine/source/ObjLoader.cpp 
	Engine/source/Random.cpp 
	Engine/source/Renderer.cpp 
	Engine/source/StreamReader.cpp 
	Engine/source/Texture.cpp 
	Engine/source/Transform.cpp 
	Engine/source/Transition.cpp 
	Engine/source/Vertex.cpp 
	Engine/source/VertexArray.cpp 
	Engine/source/Window.cpp 
	Engine/source/Script.cpp 
	Engine/source/OpenGL/ShaderGL.cpp 
	Engine/source/ES/ESRenderer.cpp 
	Components/source/AudioComponent.cpp 
	Components/source/CircleComponent.cpp 
	Components/source/Component.cpp 
	Components/source/EffectComponent.cpp 
	Components/source/FontComponent.cpp 
	Components/source/InputComponent.cpp 
	Components/source/MoveComponent.cpp 
	Components/source/Sprite2DComponent.cpp 
	Components/source/Sprite3DComponent.cpp 
	Actor/source/Actor.cpp 
	Actor/source/Animation2D.cpp 
	Actor/source/PlaneActor.cpp 
	Actor/source/Text.cpp 
	Scene/source/Scene.cpp 
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
	./Engine 
	./Engine/include 
	./Engine/include/Vulkan 
	./Engine/include/OpenGL 
	./Engine/include/ES 
	./Components 
	./Components/include 
	./Actor 
	./Actor/include 
	./lib/tinyobjloader 
	./Scene/include 
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
