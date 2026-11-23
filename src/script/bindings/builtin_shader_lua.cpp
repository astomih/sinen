#include "luaapi.hpp"
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
static auto lBuiltinShaderGetDefaultVs() {
  return makePtr<Shader>(BuiltinShader::getDefaultVS());
}
static auto lBuiltinShaderGetDefaultFs() {
  return makePtr<Shader>(BuiltinShader::getDefaultFS());
}
static auto lBuiltinShaderGetFontFs() {
  return makePtr<Shader>(BuiltinShader::getFontFS());
}
static auto lBuiltinShaderGetDefaultInstancedVs() {
  return makePtr<Shader>(BuiltinShader::getDefaultInstancedVS());
}
static auto lBuiltinShaderGetRectFs() {
  return makePtr<Shader>(BuiltinShader::getRectFS());
}
static auto lBuiltinShaderGetCubemapVs() {
  return makePtr<Shader>(BuiltinShader::getCubemapVS());
}
static auto lBuiltinShaderGetCubemapFs() {
  return makePtr<Shader>(BuiltinShader::getCubemapFS());
}
void registerBuiltinShader(luau::table &sn) {
  auto builtinShader = sn.get_or_create_table("BuiltinShader");
  builtinShader.function<lBuiltinShaderGetDefaultVs>("getDefaultVS");
  builtinShader.function<lBuiltinShaderGetDefaultFs>("getDefaultFS");
  builtinShader.function<lBuiltinShaderGetFontFs>("getFontFS");
  builtinShader.function<lBuiltinShaderGetDefaultInstancedVs>(
      "getDefaultInstancedVS");
  builtinShader.function<lBuiltinShaderGetRectFs>("getRectFS");
  builtinShader.function<lBuiltinShaderGetCubemapVs>("getCubemapVS");
  builtinShader.function<lBuiltinShaderGetCubemapFs>("getCubemapFS");
}
} // namespace sinen
