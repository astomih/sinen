file(GLOB_RECURSE SINEN_GPU_SOURCES CONFIGURE_DEPENDS
     ${SINEN_SOURCE_DIR}/src/gpu/*.cpp)
file(GLOB_RECURSE SINEN_GPU_HEADERS CONFIGURE_DEPENDS
     ${SINEN_SOURCE_DIR}/src/gpu/*.hpp ${SINEN_SOURCE_DIR}/src/gpu/*.inl)

if(NOT SINEN_ENABLE_WEBGPU)
  list(FILTER SINEN_GPU_SOURCES EXCLUDE REGEX
       ".*[\\\\/]src[\\\\/]gpu[\\\\/]webgpu[\\\\/].*\\.cpp$")
  list(FILTER SINEN_GPU_HEADERS EXCLUDE REGEX
       ".*[\\\\/]src[\\\\/]gpu[\\\\/]webgpu[\\\\/].*\\.(hpp|inl)$")
endif()

sinen_add_module(sinen_gpu ${SINEN_GPU_SOURCES} ${SINEN_GPU_HEADERS})
target_include_directories(
  sinen_gpu
  PUBLIC ${Vulkan_INCLUDE_DIR} ${SINEN_SOURCE_DIR}/libs/volk
         ${SINEN_SOURCE_DIR}/libs/VulkanMemoryAllocator/include)
target_link_libraries(sinen_gpu PUBLIC SDL3::SDL3)

if(SINEN_ENABLE_WEBGPU)
  target_link_libraries(sinen_gpu PUBLIC webgpu)
  if(TARGET dawn::webgpu_dawn)
    get_target_property(SINEN_DAWN_INCLUDE_DIRS dawn::webgpu_dawn
                        INTERFACE_INCLUDE_DIRECTORIES)
    if(SINEN_DAWN_INCLUDE_DIRS)
      target_include_directories(sinen_gpu BEFORE
                                 PUBLIC ${SINEN_DAWN_INCLUDE_DIRS})
    endif()
  endif()
endif()

if(WIN32)
  target_link_libraries(sinen_gpu PUBLIC d3d12 dxgi dxguid)
endif()
