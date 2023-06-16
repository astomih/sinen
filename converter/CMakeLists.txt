cmake_minimum_required(VERSION 3.7)
project(converter)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)


file(GLOB SOURCE_FILES
	./src/*/*.cpp
	./src/*.cpp
	../src/math/*.cpp
	../src/vertex/vertex_array.cpp
)

include_directories(
	../include
	../include/sinen
)
link_directories(
	..
)
find_package(assimp REQUIRED)

set(COMMON_LIBRARIES
  assimp
)
if(WIN32)
	set(LIBRARIES ${COMMON_LIBRARIES})
else()
	set(LIBRARIES ${COMMON_LIBRARIES})
endif()

add_executable(converter
	${SOURCE_FILES}
)
target_link_libraries(converter 
	${LIBRARIES}
)

target_compile_features(converter PUBLIC cxx_std_20)