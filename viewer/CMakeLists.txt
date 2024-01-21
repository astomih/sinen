cmake_minimum_required(VERSION 3.7)
project(viewer)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
if(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -mwindows")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS true)


file(GLOB SOURCE_FILES
	./native/*/*.cpp 
	./native/*.cpp
	../libs/ImGuiColorTextEdit/*.cpp
	../libs/ImGuizmo/*.cpp
	./data/icon/icon.rc
)

include_directories(
	../include
	../include/sinen
	../libs/imgui_markdown
	../libs/ImGuiColorTextEdit
	../libs/ImGuizmo
	../libs/rapidjson/include
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

add_executable(viewer
	${SOURCE_FILES}
)
target_link_libraries(viewer 
	${LIBRARIES}
)

target_compile_features(viewer PUBLIC cxx_std_20)
