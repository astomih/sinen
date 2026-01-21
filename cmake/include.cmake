set(SINEN_INCLUDE_DIRS
    ${Vulkan_INCLUDE_DIR}
    ${SINEN_SOURCE_DIR}
    ${SINEN_SOURCE_DIR}/src
    ${SINEN_SOURCE_DIR}/libs/imgui
    ${SINEN_SOURCE_DIR}/libs/imgui/backends
    ${SINEN_SOURCE_DIR}/libs/rapidjson/include
    ${SINEN_SOURCE_DIR}/libs/SDL/include
    ${SINEN_SOURCE_DIR}/libs/miniaudio
    ${SINEN_SOURCE_DIR}/src/script/include
    ${SINEN_SOURCE_DIR}/src/rt_shader_compiler/include
    ${SINEN_SOURCE_DIR}/libs/assimp/include
    ${SINEN_SOURCE_DIR}/libs/SDL/include
    ${SINEN_SOURCE_DIR}/libs/stb
    ${SINEN_SOURCE_DIR}/libs/slang/include
    ${SINEN_SOURCE_DIR}/libs/JoltPhysics
    ${SINEN_SOURCE_DIR}/libs/tinyexr
    ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz
    ${SINEN_SOURCE_DIR}/libs/tlsf
    ${SINEN_SOURCE_DIR}/libs/volk
    ${SINEN_SOURCE_DIR}/libs/VulkanMemoryAllocator/include)

if(SINEN_USE_LUAU)
  list(APPEND SINEN_INCLUDE_DIRS ${SINEN_SOURCE_DIR}/libs/luau/VM/include)
else()
  list(APPEND SINEN_INCLUDE_DIRS ${SINEN_SOURCE_DIR}/libs/LuaJIT/src
       ${SINEN_BINARY_DIR}/LuaJIT ${SINEN_BINARY_DIR}/LuaJIT/src)
endif()
