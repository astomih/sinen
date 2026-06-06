#include <gpu/shader/builtin_shader.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>


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
