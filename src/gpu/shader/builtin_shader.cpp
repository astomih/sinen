#include "builtin_shader.hpp"
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

#include "shader_bundle.hpp"

#include "default/cubemap.frag.snb.hpp"
#include "default/cubemap.vert.snb.hpp"
#include "default/font.frag.snb.hpp"
#include "default/rect_color.frag.snb.hpp"
#include "default/shader.frag.snb.hpp"
#include "default/shader.vert.snb.hpp"
#include "default/shader_instance.vert.snb.hpp"

namespace sinen {
static Shader defaultVS;
static Shader defaultInstancedVS;
static Shader defaultFS;
static Shader fontFS;
static Shader rectFS;
static Shader cubemapVS;
static Shader cubemapFS;

static Shader createBuiltinShader(const unsigned char *bundleData,
                                  unsigned int bundleSize, ShaderStage stage) {
  auto *allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  const auto format = ShaderBundle::preferredFormatFor(device->getBackendAPI());
  auto entry = ShaderBundle::select(
      StringView(reinterpret_cast<const char *>(bundleData), bundleSize), stage,
      format);
  if (!entry) {
    return Shader();
  }

  gpu::Shader::CreateInfo info{};
  info.allocator = allocator;
  info.size = entry->code.size();
  info.data = entry->code.data();
  info.entrypoint = ShaderBundle::entryPointFor(entry->stage, entry->format);
  info.format = entry->format;
  info.stage = entry->stage;
  info.numSamplers = entry->numSamplers;
  info.numStorageBuffers = entry->numStorageBuffers;
  info.numStorageTextures = entry->numStorageTextures;
  info.numUniformBuffers = entry->numUniformBuffers;
  return Shader(device->createShader(info));
}

bool BuiltinShader::initialize() {
  defaultVS = createBuiltinShader(shader_vert_snb, shader_vert_snb_len,
                                  ShaderStage::Vertex);
  defaultInstancedVS =
      createBuiltinShader(shader_instance_vert_snb,
                          shader_instance_vert_snb_len, ShaderStage::Vertex);
  defaultFS = createBuiltinShader(shader_frag_snb, shader_frag_snb_len,
                                  ShaderStage::Fragment);
  fontFS = createBuiltinShader(font_frag_snb, font_frag_snb_len,
                               ShaderStage::Fragment);
  rectFS = createBuiltinShader(rect_color_frag_snb, rect_color_frag_snb_len,
                               ShaderStage::Fragment);
  cubemapVS = createBuiltinShader(cubemap_vert_snb, cubemap_vert_snb_len,
                                  ShaderStage::Vertex);
  cubemapFS = createBuiltinShader(cubemap_frag_snb, cubemap_frag_snb_len,
                                  ShaderStage::Fragment);

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
