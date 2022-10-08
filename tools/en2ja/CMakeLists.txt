cmake_minimum_required(VERSION 3.7)
project(en2ja)


set(CMAKE_CXX_FLAGS "-std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -pg -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNDEBUG -march=native -W2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++")
file(GLOB RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ejdict-hand-utf8.txt)
file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB SOURCE_FILES
	./src/*/*.cpp 
	./src/*.cpp
)


add_executable(en2ja
	${SOURCE_FILES}
	${CMAKE_}
)
