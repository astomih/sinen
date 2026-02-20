#include "builtin_shader.hpp"
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

#include "default/cubemap.frag.spv.hpp"
#include "default/cubemap.vert.spv.hpp"
#include "default/rect_color.frag.spv.hpp"
#include "default/shader.frag.spv.hpp"
#include "default/shader.vert.spv.hpp"
#include "default/shader_instance.vert.spv.hpp"

namespace sinen {
static Shader defaultVS;
static Shader defaultInstancedVS;
static Shader defaultFS;
static Shader rectFS;
static Shader cubemapVS;
static Shader cubemapFS;

bool BuiltinShader::initialize() {
  {

    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    vsInfo.size = shader_vert_spv_len;
    vsInfo.data = shader_vert_spv;
    vsInfo.entrypoint = "main";
    vsInfo.format = gpu::ShaderFormat::SPIRV;
    vsInfo.stage = gpu::ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    defaultVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    vsInfo.size = shader_instance_vert_spv_len;
    vsInfo.data = shader_instance_vert_spv;
    vsInfo.entrypoint = "main";
    vsInfo.format = gpu::ShaderFormat::SPIRV;
    vsInfo.stage = gpu::ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    defaultInstancedVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    fsInfo.size = shader_frag_spv_len;
    fsInfo.data = shader_frag_spv;
    fsInfo.entrypoint = "main";
    fsInfo.format = gpu::ShaderFormat::SPIRV;
    fsInfo.stage = gpu::ShaderStage::Fragment;
    fsInfo.numSamplers = 1; // one sampler for fragment shader
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
    defaultFS = Shader(device->createShader(fsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    fsInfo.size = rect_color_frag_spv_len;
    fsInfo.data = rect_color_frag_spv;
    fsInfo.entrypoint = "main";
    fsInfo.format = gpu::ShaderFormat::SPIRV;
    fsInfo.stage = gpu::ShaderStage::Fragment;
    fsInfo.numSamplers = 0; // no sampler for solid-color rectangle
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 2;
    rectFS = Shader(device->createShader(fsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    vsInfo.size = cubemap_vert_spv_len;
    vsInfo.data = cubemap_vert_spv;
    vsInfo.entrypoint = "main";
    vsInfo.format = gpu::ShaderFormat::SPIRV;
    vsInfo.stage = gpu::ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    cubemapVS = Shader(device->createShader(vsInfo));
  }
  {
    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    fsInfo.size = cubemap_frag_spv_len;
    fsInfo.data = cubemap_frag_spv;
    fsInfo.entrypoint = "main";
    fsInfo.format = gpu::ShaderFormat::SPIRV;
    fsInfo.stage = gpu::ShaderStage::Fragment;
    fsInfo.numSamplers = 1; // one sampler for fragment shader
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
    cubemapFS = Shader(device->createShader(fsInfo));
  }

  return true;
}
void BuiltinShader::shutdown() {}

Shader BuiltinShader::getDefaultVS() { return defaultVS; }
Shader BuiltinShader::getDefaultInstancedVS() { return defaultInstancedVS; }
Shader BuiltinShader::getDefaultFS() { return defaultFS; }
Shader BuiltinShader::getRectFS() { return rectFS; }
Shader BuiltinShader::getCubemapVS() { return cubemapVS; }
Shader BuiltinShader::getCubemapFS() { return cubemapFS; }

static int lBuiltinShaderGetDefaultVs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getDefaultVS()));
  return 1;
}
static int lBuiltinShaderGetDefaultFs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getDefaultFS()));
  return 1;
}
static int lBuiltinShaderGetDefaultInstancedVs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getDefaultInstancedVS()));
  return 1;
}
static int lBuiltinShaderGetRectFs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getRectFS()));
  return 1;
}
static int lBuiltinShaderGetCubemapVs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getCubemapVS()));
  return 1;
}
static int lBuiltinShaderGetCubemapFs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getCubemapFS()));
  return 1;
}
void registerBuiltinShader(lua_State *L) {
  pushSnNamed(L, "BuiltinShader");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultVs);
  lua_setfield(L, -2, "getDefaultVS");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultFs);
  lua_setfield(L, -2, "getDefaultFS");
  luaPushcfunction2(L, lBuiltinShaderGetDefaultInstancedVs);
  lua_setfield(L, -2, "getDefaultInstancedVS");
  luaPushcfunction2(L, lBuiltinShaderGetRectFs);
  lua_setfield(L, -2, "getRectFS");
  luaPushcfunction2(L, lBuiltinShaderGetCubemapVs);
  lua_setfield(L, -2, "getCubemapVS");
  luaPushcfunction2(L, lBuiltinShaderGetCubemapFs);
  lua_setfield(L, -2, "getCubemapFS");
  lua_pop(L, 1);
}
} // namespace sinen
