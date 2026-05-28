sinen_glob_module(sinen_shader_compiler shader_compiler)

if(SINEN_USE_SLANG)
  target_link_libraries(sinen_shader_compiler PUBLIC slang)
endif()
