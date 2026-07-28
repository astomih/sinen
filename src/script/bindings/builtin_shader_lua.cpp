#include "luaapi.hpp"
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
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
  Binding::registerFunction(L, "getDefaultVS", lBuiltinShaderGetDefaultVs);
  Binding::registerFunction(L, "getDefaultFS", lBuiltinShaderGetDefaultFs);
  Binding::registerFunction(L, "getFontFS", lBuiltinShaderGetFontFs);
  Binding::registerFunction(L, "getDefaultInstancedVS",
                            lBuiltinShaderGetDefaultInstancedVs);
  Binding::registerFunction(L, "getRectFS", lBuiltinShaderGetRectFs);
  Binding::registerFunction(L, "getCubemapVS", lBuiltinShaderGetCubemapVs);
  Binding::registerFunction(L, "getCubemapFS", lBuiltinShaderGetCubemapFs);
  lua_pop(L, 1);
}
} // namespace sinen
