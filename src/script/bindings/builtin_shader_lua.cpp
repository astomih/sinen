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
void registerBuiltinShader(lua_State *L) {
  pushSnNamed(L, "BuiltinShader");
  luau::table(L, -1).function("getDefaultVS", lBuiltinShaderGetDefaultVs);
  luau::table(L, -1).function("getDefaultFS", lBuiltinShaderGetDefaultFs);
  luau::table(L, -1).function("getFontFS", lBuiltinShaderGetFontFs);
  luau::table(L, -1).function("getDefaultInstancedVS",
                              lBuiltinShaderGetDefaultInstancedVs);
  luau::table(L, -1).function("getRectFS", lBuiltinShaderGetRectFs);
  luau::table(L, -1).function("getCubemapVS", lBuiltinShaderGetCubemapVs);
  luau::table(L, -1).function("getCubemapFS", lBuiltinShaderGetCubemapFs);
  lua_pop(L, 1);
}
} // namespace sinen
