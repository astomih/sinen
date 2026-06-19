function(sinen_configure_platform_app target)
  find_library(ANDROID_LIB android)
  target_link_libraries(${target} PUBLIC ${ANDROID_LIB})
endfunction()
