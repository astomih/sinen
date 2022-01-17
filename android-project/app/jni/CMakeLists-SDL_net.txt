cmake_minimum_required(VERSION 3.1.0)
project(SDL2_net C)
find_library(SDL2 SDL2)
add_library(SDL2_net SHARED
		SDLnet.c
		SDLnetTCP.c
		SDLnetselect.c)
target_link_libraries(SDL2_net SDL2)
target_include_directories(SDL2_net PUBLIC
		../SDL2/include
		)

