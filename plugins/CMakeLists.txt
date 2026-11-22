if(SINEN_PLUGIN_NATIVEFS)
  add_subdirectory(nativefs)
endif()

if(SINEN_PLUGIN_OCEAN)
  add_subdirectory(ocean)
endif()

if(SINEN_MODULE_SCRIPT AND NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(luau_to_cpp)
endif()

if(SINEN_PLUGIN_SHADER_COMPILER)
  if(EMSCRIPTEN)
    message(FATAL_ERROR "The shader compiler plugin is not supported on Emscripten")
  endif()
  add_subdirectory(shader_compiler)
endif()
