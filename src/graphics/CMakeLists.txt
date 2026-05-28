sinen_glob_module(sinen_graphics graphics)
target_include_directories(
  sinen_graphics
  PUBLIC ${SINEN_SOURCE_DIR}/libs/stb ${SINEN_SOURCE_DIR}/libs/tinyexr
         ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz)
target_link_libraries(sinen_graphics PUBLIC SDL3::SDL3 assimp meshoptimizer
                                            ktx_read msdfgen-core)
