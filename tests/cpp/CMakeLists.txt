add_subdirectory(googletest EXCLUDE_FROM_ALL)

add_executable(sinen_allocator_tests allocator_tests.cpp)
target_link_libraries(sinen_allocator_tests PRIVATE sinen_core GTest::gtest_main)
set_property(TARGET sinen_allocator_tests PROPERTY CXX_STANDARD 23)

add_test(NAME sinen_allocator_tests COMMAND sinen_allocator_tests)

add_executable(sinen_license_report_tests license_report_tests.cpp)
target_link_libraries(sinen_license_report_tests
                      PRIVATE sinen_core GTest::gtest_main)
set_property(TARGET sinen_license_report_tests PROPERTY CXX_STANDARD 23)

add_test(NAME sinen_license_report_tests COMMAND sinen_license_report_tests)

if(TARGET sinen_renderer)
  add_executable(sinen_global_illumination_tests global_illumination_tests.cpp)
  if(WIN32)
    target_sources(sinen_global_illumination_tests PRIVATE realtime_gi_tests.cpp)
    target_include_directories(sinen_global_illumination_tests PRIVATE ${SINEN_SOURCE_DIR}/src)
  endif()
  target_link_libraries(sinen_global_illumination_tests PRIVATE sinen_renderer GTest::gtest_main)
  if(TARGET sinen_script)
    target_link_libraries(sinen_global_illumination_tests PRIVATE sinen_script)
  endif()
  set_property(TARGET sinen_global_illumination_tests PROPERTY CXX_STANDARD 23)
  add_test(NAME sinen_global_illumination_tests COMMAND sinen_global_illumination_tests)
  if(WIN32 AND SINEN_ENABLE_WEBGPU)
    # Dawn's D3D12 backend needs DXC at runtime. Reuse the shader compiler's
    # downloaded package when available without installing it system-wide.
    FetchContent_GetProperties(dxc SOURCE_DIR SINEN_GI_TEST_DXC_SOURCE)
    if(SINEN_GI_TEST_DXC_SOURCE)
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(SINEN_GI_TEST_DXC_ARCH arm64)
      else()
        set(SINEN_GI_TEST_DXC_ARCH x64)
      endif()
      set_tests_properties(sinen_global_illumination_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${SINEN_GI_TEST_DXC_SOURCE}/bin/${SINEN_GI_TEST_DXC_ARCH}")
    endif()
  endif()
endif()

if(TARGET sinen_graphics)
  add_executable(sinen_dds_decoder_tests dds_decoder_tests.cpp)
  target_include_directories(sinen_dds_decoder_tests PRIVATE ${SINEN_SOURCE_DIR}/src)
  target_link_libraries(sinen_dds_decoder_tests PRIVATE GTest::gtest_main)
  set_property(TARGET sinen_dds_decoder_tests PROPERTY CXX_STANDARD 23)
  add_test(NAME sinen_dds_decoder_tests COMMAND sinen_dds_decoder_tests)
  if(NOT EMSCRIPTEN)
    # Decode generated H.264 in native tests, keeping decoder code out of the engine.
    # The upstream decoder uses sem_timedwait, absent in single-threaded Wasm.
    file(GLOB SINEN_TEST_H264_DECODER CONFIGURE_DEPENDS
         ${SINEN_OPENH264_DIR}/codec/decoder/core/src/*.cpp
         ${SINEN_OPENH264_DIR}/codec/decoder/plus/src/*.cpp)
    add_library(sinen_test_h264_decoder STATIC ${SINEN_TEST_H264_DECODER})
    target_include_directories(sinen_test_h264_decoder PRIVATE
         ${SINEN_OPENH264_DIR}/codec/api/wels
         ${SINEN_OPENH264_DIR}/codec/common/inc
         ${SINEN_OPENH264_DIR}/codec/decoder/core/inc
         ${SINEN_OPENH264_DIR}/codec/decoder/plus/inc)
    target_link_libraries(sinen_test_h264_decoder PUBLIC sinen_openh264)
  endif()
  add_executable(sinen_video_tests video_tests.cpp)
  target_link_libraries(sinen_video_tests PRIVATE sinen_graphics
                        sinen_openh264 GTest::gtest_main)
  if(TARGET sinen_test_h264_decoder)
    target_link_libraries(sinen_video_tests PRIVATE sinen_test_h264_decoder)
  endif()
  target_include_directories(sinen_video_tests PRIVATE
                             ${SINEN_SOURCE_DIR}/libs/minimp4)
  set_property(TARGET sinen_video_tests PROPERTY CXX_STANDARD 23)
  add_test(NAME sinen_video_tests COMMAND sinen_video_tests)
endif()

if(TARGET sinen_platform)
  add_executable(sinen_external_file_tests external_file_tests.cpp)
  target_link_libraries(sinen_external_file_tests
                        PRIVATE sinen_platform GTest::gtest_main)
  set_property(TARGET sinen_external_file_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_external_file_tests COMMAND sinen_external_file_tests)
  if(WIN32)
    add_executable(sinen_gamepad_tests gamepad_tests.cpp)
    target_link_libraries(sinen_gamepad_tests PRIVATE sinen_platform GTest::gtest_main)
    set_property(TARGET sinen_gamepad_tests PROPERTY CXX_STANDARD 23)
    add_test(NAME sinen_gamepad_tests COMMAND sinen_gamepad_tests)
    set_tests_properties(sinen_gamepad_tests PROPERTIES ENVIRONMENT
      "SDL_XINPUT_ENABLED=0;SDL_JOYSTICK_HIDAPI=0;SDL_JOYSTICK_RAWINPUT=0;SDL_JOYSTICK_WGI=0;SDL_JOYSTICK_DIRECTINPUT=0")
  endif()
endif()

if(TARGET sinen_gpu)
  if(WIN32)
    add_executable(sinen_d3d12_frame_tests d3d12_frame_tests.cpp)
    target_include_directories(sinen_d3d12_frame_tests PRIVATE ${SINEN_SOURCE_DIR}/src)
    target_link_libraries(sinen_d3d12_frame_tests PRIVATE sinen_gpu GTest::gtest_main)
    set_property(TARGET sinen_d3d12_frame_tests PROPERTY CXX_STANDARD 23)
    add_test(NAME sinen_d3d12_frame_tests COMMAND sinen_d3d12_frame_tests)
    set_tests_properties(sinen_d3d12_frame_tests PROPERTIES TIMEOUT 30)
  endif()
  if(WIN32 OR (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    add_library(sinen_test_renderdoc SHARED renderdoc_fixture.cpp)
    target_include_directories(sinen_test_renderdoc PRIVATE
                               ${SINEN_SOURCE_DIR}/libs/renderdoc)
    set_property(TARGET sinen_test_renderdoc PROPERTY CXX_STANDARD 23)
    add_executable(sinen_renderdoc_tests renderdoc_tests.cpp)
    target_link_libraries(sinen_renderdoc_tests PRIVATE sinen_gpu GTest::gtest_main)
    target_compile_definitions(sinen_renderdoc_tests PRIVATE
      SINEN_TEST_RENDERDOC_PATH="$<TARGET_FILE:sinen_test_renderdoc>")
    set_property(TARGET sinen_renderdoc_tests PROPERTY CXX_STANDARD 23)
    add_dependencies(sinen_renderdoc_tests sinen_test_renderdoc)
    add_test(NAME sinen_renderdoc_tests COMMAND sinen_renderdoc_tests)
  endif()
  add_executable(sinen_shader_bundle_tests shader_bundle_tests.cpp)
  target_link_libraries(sinen_shader_bundle_tests
                        PRIVATE sinen_gpu GTest::gtest_main)
  target_compile_definitions(
    sinen_shader_bundle_tests
    PRIVATE SINEN_TEST_SOURCE_DIR="${SINEN_SOURCE_DIR}")
  set_property(TARGET sinen_shader_bundle_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_shader_bundle_tests COMMAND sinen_shader_bundle_tests)
endif()

if(TARGET sinen_script)
  if(WIN32)
    add_executable(sinen_text_cache_tests text_cache_tests.cpp)
    target_include_directories(sinen_text_cache_tests PRIVATE ${SINEN_SOURCE_DIR}/src)
    target_link_libraries(sinen_text_cache_tests PRIVATE sinen_script GTest::gtest_main)
    set_property(TARGET sinen_text_cache_tests PROPERTY CXX_STANDARD 23)
    add_test(NAME sinen_text_cache_tests COMMAND sinen_text_cache_tests)
    set_tests_properties(sinen_text_cache_tests PROPERTIES TIMEOUT 60)
  endif()
  if(WIN32 OR EMSCRIPTEN)
    sinen_add_lua_module(sinen_test_native_module native_module_fixture.cpp)
  endif()
  if(WIN32)
    sinen_add_lua_module(sinen_test_failing_native_module
                         native_module_failure_fixture.cpp)
  endif()

  add_executable(
    sinen_luau_binding_tests
    luau_binding_tests.cpp
    luau_bind_library_tests.cpp
    ${SINEN_SOURCE_DIR}/src/script/bindings/binding_registration.cpp
    ${SINEN_SOURCE_DIR}/src/script/require_alias.cpp)
  if(WIN32)
    target_sources(
      sinen_luau_binding_tests
      PRIVATE native_module_tests.cpp
              ${SINEN_SOURCE_DIR}/src/script/native_module.cpp)
    target_compile_definitions(
      sinen_luau_binding_tests
      PRIVATE
        SINEN_TEST_NATIVE_MODULE_PATH="$<TARGET_FILE:sinen_test_native_module>")
    target_compile_definitions(
      sinen_luau_binding_tests
      PRIVATE
        SINEN_TEST_FAILING_NATIVE_MODULE_PATH="$<TARGET_FILE:sinen_test_failing_native_module>")
    add_dependencies(sinen_luau_binding_tests sinen_test_native_module
                     sinen_test_failing_native_module)
    if(TARGET sinen_plugin_luau_to_cpp)
      target_compile_definitions(
        sinen_luau_binding_tests PRIVATE
        SINEN_TEST_LUAU_TO_CPP_DIR="$<TARGET_FILE_DIR:sinen_plugin_luau_to_cpp>")
      add_dependencies(sinen_luau_binding_tests sinen_plugin_luau_to_cpp)
    endif()
    if(TARGET sinen_plugin_fps_collision)
      target_compile_definitions(
        sinen_luau_binding_tests PRIVATE
        SINEN_TEST_FPS_DIR="${SINEN_SOURCE_DIR}/game/fps")
      add_dependencies(sinen_luau_binding_tests sinen_deploy_fps_collision)
    endif()
    if(TARGET sinen_plugin_nativefs)
      target_compile_definitions(
        sinen_luau_binding_tests
        PRIVATE
          SINEN_TEST_NATIVEFS_PATH="$<TARGET_FILE:sinen_plugin_nativefs>"
          SINEN_TEST_SOURCE_DIR="${SINEN_SOURCE_DIR}")
      add_dependencies(sinen_luau_binding_tests sinen_plugin_nativefs)
    endif()
    if(TARGET sinen_plugin_shader_compiler)
      target_compile_definitions(
        sinen_luau_binding_tests
        PRIVATE
          SINEN_TEST_SHADER_COMPILER_PLUGIN_PATH="$<TARGET_FILE:sinen_plugin_shader_compiler>")
      add_dependencies(sinen_luau_binding_tests sinen_plugin_shader_compiler)
    endif()
  endif()
  target_include_directories(sinen_luau_binding_tests
                             PRIVATE ${SINEN_SOURCE_DIR}/src
                                     ${SINEN_SOURCE_DIR}/include/sinen)
  target_link_libraries(sinen_luau_binding_tests
                        PRIVATE Luau.VM Luau.Compiler GTest::gtest_main)
  set_property(TARGET sinen_luau_binding_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_luau_binding_tests COMMAND sinen_luau_binding_tests)
  set_tests_properties(sinen_luau_binding_tests
                       PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
