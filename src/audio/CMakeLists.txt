sinen_glob_module(sinen_audio audio)
target_include_directories(sinen_audio PUBLIC ${SINEN_SOURCE_DIR}/libs/miniaudio)
target_link_libraries(sinen_audio PUBLIC SDL3::SDL3)
