cmake_minimum_required(VERSION 3.7)
project(editor)

set(SINEN_SOURCE_DIR ${SINEN_SOURCE_DIR} PARENT_SCOPE)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif()

if(WIN32)
	if(ALL_BUILD)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -mwindows")
	endif()
endif()

set(CMAKE_EXPORT_COFMPILE_COMMANDS true)

file(GLOB_RECURSE SOURCE_FILES
	./native/*.cpp
	${SINEN_SOURCE_DIR}/libs/ImGuiColorTextEdit/*.cpp
	${SINEN_SOURCE_DIR}/libs/ImGuizmo/*.cpp
	./data/icon/icon.rc
)

link_directories(
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
target_include_directories(editor
	PUBLIC
	${SINEN_SOURCE_DIR}/include
	${SINEN_SOURCE_DIR}/include/sinen
	${SINEN_SOURCE_DIR}/libs/imgui_markdown
	${SINEN_SOURCE_DIR}/libs/ImGuiColorTextEdit
	${SINEN_SOURCE_DIR}/libs/ImGuizmo
	${SINEN_SOURCE_DIR}/libs/rapidjson/include
)
target_link_libraries(editor
	${LIBRARIES}
)

target_compile_features(editor PUBLIC cxx_std_20)
