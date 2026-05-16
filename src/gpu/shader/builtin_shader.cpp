#include "builtin_shader.hpp"
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

// SPIR-V
#include "default/cubemap.frag.spv.hpp"
#include "default/cubemap.vert.spv.hpp"
#include "default/font.frag.spv.hpp"
#include "default/rect_color.frag.spv.hpp"
#include "default/shader.frag.spv.hpp"
#include "default/shader.vert.spv.hpp"
#include "default/shader_instance.vert.spv.hpp"

// WGSL
#include "default/cubemap.frag.wgsl.hpp"
#include "default/cubemap.vert.wgsl.hpp"
#include "default/font.frag.wgsl.hpp"
#include "default/rect_color.frag.wgsl.hpp"
#include "default/shader.frag.wgsl.hpp"
#include "default/shader.vert.wgsl.hpp"
#include "default/shader_instance.vert.wgsl.hpp"

// DXIL
#include "default/cubemap.frag.dxil.hpp"
#include "default/cubemap.vert.dxil.hpp"
#include "default/font.frag.dxil.hpp"
#include "default/rect_color.frag.dxil.hpp"
#include "default/shader.frag.dxil.hpp"
#include "default/shader.vert.dxil.hpp"
#include "default/shader_instance.vert.dxil.hpp"

namespace sinen {
static Shader defaultVS;
static Shader defaultInstancedVS;
static Shader defaultFS;
static Shader fontFS;
static Shader rectFS;
static Shader cubemapVS;
static Shader cubemapFS;

static const char *entryPointFor(ShaderStage stage, ShaderFormat format) {
  if (format != ShaderFormat::WGSL) {
    return "main";
  }
  return stage == ShaderStage::Vertex ? "VSMain" : "FSMain";
}

static void setShaderCode(gpu::Shader::CreateInfo &info, ShaderFormat format,
                          const void *spvData, size_t spvSize,
                          const void *wgslData, size_t wgslSize,
                          const void *dxilData, size_t dxilSize) {
  if (format == ShaderFormat::WGSL) {
    info.size = wgslSize;
    info.data = wgslData;
  } else if (format == ShaderFormat::DXIL) {
    info.size = dxilSize;
    info.data = dxilData;
  } else {
    info.size = spvSize;
    info.data = spvData;
  }
}

bool BuiltinShader::initialize() {
  auto *allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  auto format = ShaderFormat::SPIRV;
  if (device->getBackendAPI() == GPUBackendAPI::WebGPU) {
    format = ShaderFormat::WGSL;
  }
#ifdef SINEN_PLATFORM_WINDOWS
  if (device->getBackendAPI() == GPUBackendAPI::D3D12) {
    format = ShaderFormat::DXIL;
  }
#endif
  {
    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    setShaderCode(vsInfo, format, shader_vert_spv, shader_vert_spv_len,
                  shader_vert_wgsl, shader_vert_wgsl_len, shader_vert_dxil,
                  shader_vert_dxil_len);
    vsInfo.entrypoint = entryPointFor(ShaderStage::Vertex, format);
    vsInfo.format = format;
    vsInfo.stage = ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    defaultVS = Shader(device->createShader(vsInfo));
  }
  {
    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    setShaderCode(vsInfo, format, shader_instance_vert_spv,
                  shader_instance_vert_spv_len, shader_instance_vert_wgsl,
                  shader_instance_vert_wgsl_len, shader_instance_vert_dxil,
                  shader_instance_vert_dxil_len);
    vsInfo.entrypoint = entryPointFor(ShaderStage::Vertex, format);
    vsInfo.format = format;
    vsInfo.stage = ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    defaultInstancedVS = Shader(device->createShader(vsInfo));
  }
  {
    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    setShaderCode(fsInfo, format, shader_frag_spv, shader_frag_spv_len,
                  shader_frag_wgsl, shader_frag_wgsl_len, shader_frag_dxil,
                  shader_frag_dxil_len);
    fsInfo.entrypoint = entryPointFor(ShaderStage::Fragment, format);
    fsInfo.format = format;
    fsInfo.stage = ShaderStage::Fragment;
    fsInfo.numSamplers = 1; // one sampler for fragment shader
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 1; // only one uniform buffer for fragment shader
    defaultFS = Shader(device->createShader(fsInfo));
  }
  {
    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    setShaderCode(fsInfo, format, font_frag_spv, font_frag_spv_len,
                  font_frag_wgsl, font_frag_wgsl_len, font_frag_dxil,
                  font_frag_dxil_len);
    fsInfo.entrypoint = entryPointFor(ShaderStage::Fragment, format);
    fsInfo.format = format;
    fsInfo.stage = ShaderStage::Fragment;
    fsInfo.numSamplers = 1;
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 2;
    fontFS = Shader(device->createShader(fsInfo));
  }
  {
    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    setShaderCode(fsInfo, format, rect_color_frag_spv, rect_color_frag_spv_len,
                  rect_color_frag_wgsl, rect_color_frag_wgsl_len,
                  rect_color_frag_dxil, rect_color_frag_dxil_len);
    fsInfo.entrypoint = entryPointFor(ShaderStage::Fragment, format);
    fsInfo.format = format;
    fsInfo.stage = ShaderStage::Fragment;
    fsInfo.numSamplers = 0; // no sampler for solid-color rectangle
    fsInfo.numStorageBuffers = 0;
    fsInfo.numStorageTextures = 0;
    fsInfo.numUniformBuffers = 2;
    rectFS = Shader(device->createShader(fsInfo));
  }
  {
    gpu::Shader::CreateInfo vsInfo{};
    vsInfo.allocator = allocator;
    setShaderCode(vsInfo, format, cubemap_vert_spv, cubemap_vert_spv_len,
                  cubemap_vert_wgsl, cubemap_vert_wgsl_len, cubemap_vert_dxil,
                  cubemap_vert_dxil_len);
    vsInfo.entrypoint = entryPointFor(ShaderStage::Vertex, format);
    vsInfo.format = format;
    vsInfo.stage = ShaderStage::Vertex;
    vsInfo.numSamplers = 0;
    vsInfo.numStorageBuffers = 0;
    vsInfo.numStorageTextures = 0;
    vsInfo.numUniformBuffers = 1; // only one uniform buffer for vertex shader
    cubemapVS = Shader(device->createShader(vsInfo));
  }
  {
    gpu::Shader::CreateInfo fsInfo{};
    fsInfo.allocator = allocator;
    setShaderCode(fsInfo, format, cubemap_frag_spv, cubemap_frag_spv_len,
                  cubemap_frag_wgsl, cubemap_frag_wgsl_len, cubemap_frag_dxil,
                  cubemap_frag_dxil_len);
    fsInfo.entrypoint = entryPointFor(ShaderStage::Fragment, format);
    fsInfo.format = format;
    fsInfo.stage = ShaderStage::Fragment;
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
Shader BuiltinShader::getFontFS() { return fontFS; }
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
static int lBuiltinShaderGetFontFs(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>(BuiltinShader::getFontFS()));
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
  luaPushcfunction2(L, lBuiltinShaderGetFontFs);
  lua_setfield(L, -2, "getFontFS");
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
