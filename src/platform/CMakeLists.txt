sinen_glob_module(sinen_platform platform)
target_sources(sinen_platform PRIVATE
               ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz/miniz.c)
target_include_directories(sinen_platform
                           PUBLIC ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz)
target_link_libraries(sinen_platform PUBLIC SDL3::SDL3)
