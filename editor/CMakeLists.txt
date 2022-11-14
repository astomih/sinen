cmake_minimum_required(VERSION 3.7)
project(editor)
set(CMAKE_CXX_FLAGS "-std=c++2a")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)


file(GLOB SOURCE_FILES
	./src/*/*.cpp 
	./src/*.cpp
  ../external/ImGuiColorTextEdit/*.cpp
  ../external/ImGuizmo/*.cpp
)

include_directories(
	../include
	../include/sinen
	../external/imgui_markdown
	../external/imgui-filebrowser
  ../external/ImGuiColorTextEdit
	../external/ImGuizmo
	../external/rapidjson/include
)
link_directories(
	..
)
find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

set(COMMON_LIBRARIES
	sinen
	${Vulkan_LIBRARIES}	
	SDL2
	SDL2main
	SDL2_image
	SDL2_mixer
	SDL2_ttf
	SDL2_net
)
if(WIN32)
	set(LIBRARIES ${COMMON_LIBRARIES} glew32 opengl32)
else()
	set(LIBRARIES ${COMMON_LIBRARIES} GLEW GL ${CMAKE_DL_LIBS})
endif()

add_executable(editor
	${SOURCE_FILES}
)
target_link_libraries(editor 
	${LIBRARIES}
)
