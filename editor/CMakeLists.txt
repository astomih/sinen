cmake_minimum_required(VERSION 3.7)
project(editor)
set(CMAKE_CXX_FLAGS "-std=c++2a")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
if(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -mwindows")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS true)


file(GLOB SOURCE_FILES
	./native/*/*.cpp 
	./native/*.cpp
  ../external/ImGuiColorTextEdit/*.cpp
  ../external/ImGuizmo/*.cpp
	./data/icon/icon.rc
)

include_directories(
	../include
	../include/sinen
	../external/imgui_markdown
  ../external/ImGuiColorTextEdit
	../external/ImGuizmo
	../external/rapidjson/include
)
link_directories(
	..
)
find_package(Vulkan REQUIRED)

set(COMMON_LIBRARIES
	sinen
	script_engine
	${Vulkan_LIBRARIES}	
)
if(WIN32)
	set(LIBRARIES ${COMMON_LIBRARIES})
else()
	set(LIBRARIES ${COMMON_LIBRARIES} ${CMAKE_DL_LIBS})
endif()

add_executable(editor
	${SOURCE_FILES}
)
target_link_libraries(editor 
	${LIBRARIES}
)
