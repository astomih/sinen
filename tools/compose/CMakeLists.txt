cmake_minimum_required(VERSION 3.7)
project(nem)
set(CMAKE_CXX_FLAGS "-std=c++2a")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
find_package(OpenAL REQUIRED)
find_package(boost REQUIRED)


file(GLOB SOURCE_FILES
	./src/*/*.cpp 
	./src/*.cpp
)

include_directories(
)
link_directories(
)
set(LIBRARIES
	openal
)

add_executable(nem
	${SOURCE_FILES}
)
target_link_libraries(nem 
	${LIBRARIES}
)
