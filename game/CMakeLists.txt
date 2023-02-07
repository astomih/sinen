cmake_minimum_required(VERSION 3.7)
project(game)
set(CMAKE_CXX_FLAGS "-std=c++2a")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS true)


file(GLOB SOURCE_FILES
	./src/*/*.cpp 
	./src/*.cpp
	./icon.rc
)
file(GLOB INCLUDE_DIRS
	./include
	./include/sinen
)
if(EMSCRIPTEN)
  set(CMAKE_C_COMPILER "emcc")
  add_executable(index ${SOURCE_FILES})
  target_link_directories(index PUBLIC .)
  target_link_libraries(index 
        sinen
        script_engine
  )
  target_include_directories(index PUBLIC ${INCLUDE_DIRS})
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} \
		-Wno-unused-command-line-argument \
		-s USE_SDL=2 \
		-s USE_SDL_IMAGE=2 \
		-s USE_WEBGL2=1 \
		-s FULL_ES3=1 \
		-s MIN_WEBGL_VERSION=2 \
		-s MAX_WEBGL_VERSION=2 \
		-s USE_SDL_TTF=2 \
		-s USE_SDL_MIXER=2 \
		-s USE_SDL_NET=2 \
		--preload-file=data \
		-s ALLOW_MEMORY_GROWTH \
		--shell-file shell_minimum.html \
		-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
		-std=c++20")
  if(CMAKE_HOST_WIN32)
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-s SDL2_IMAGE_FORMATS="[""png""]" \
    ")
  else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} \
		-s SDL2_IMAGE_FORMATS='[\"png\"]' \
		")
  endif()
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
include_directories(
	${INCLUDE_DIRS}
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
	script_engine
	${Vulkan_LIBRARIES}	
	SDL2
	SDL2main
)
if(WIN32)
	set(LIBRARIES ${COMMON_LIBRARIES} glew32 opengl32)
else()
	set(LIBRARIES ${COMMON_LIBRARIES} GLEW GL ${CMAKE_DL_LIBS})
endif()

add_executable(game
	${SOURCE_FILES}
)
target_link_libraries(game 
	${LIBRARIES}
)
endif()
