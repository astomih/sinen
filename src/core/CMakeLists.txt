sinen_glob_module(sinen_core core)
target_sources(sinen_core PRIVATE ${SINEN_SOURCE_DIR}/libs/tlsf/tlsf.c)
target_include_directories(
  sinen_core
  PUBLIC ${SINEN_SOURCE_DIR}/libs/rapidjson/include
         ${SINEN_SOURCE_DIR}/libs/tlsf)
target_link_libraries(sinen_core PUBLIC SDL3::SDL3)
if(SINEN_MODULE_PROFILER)
  target_link_libraries(sinen_core PUBLIC Tracy::TracyClient)
endif()
