cmake_minimum_required(VERSION 3.7)
project(works)
set(CMAKE_CXX_FLAGS "-std=c++20")

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif()

if(WIN32)
	if(ALL_BUILD)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -static -mwindows")
	endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

file(GLOB SOURCE_FILES
	./native/*/*.cpp
	./native/*.cpp
	./data/icon/icon.rc
)
file(GLOB INCLUDE_DIRS
	./include
	${SINEN_SOURCE_DIR}/include
	${SINEN_SOURCE_DIR}/include/sinen
	${SINEN_SOURCE_DIR}/libs/sol2/include
)
include_directories(
	${INCLUDE_DIRS}
)
link_directories(
	${SINEN_SOURCE_DIR}
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

add_executable(works
	${SOURCE_FILES}
)
target_link_libraries(works
	${LIBRARIES}
)
