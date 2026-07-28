#include "luaapi.hpp"
#include <math/vec2.hpp>

namespace sinen {
static int lVec2New(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec2>(L, Vec2(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec2>(L, Vec2(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  udNewOwned<Vec2>(L, Vec2(x, y));
  return 1;
}
static int lVec2Index(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, v.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, v.y);
    return 1;
  }
  luaL_getmetatable(L, Vec2::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lVec2Newindex(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  float value = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    v.x = value;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    v.y = value;
    return 0;
  }
  return luaLError2(L, "sn.Vec2: invalid field '%s'", k);
}
static int lVec2Add(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1);
  auto &b = udValue<Vec2>(L, 2);
  udNewOwned<Vec2>(L, a + b);
  return 1;
}
static int lVec2Sub(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1);
  auto &b = udValue<Vec2>(L, 2);
  udNewOwned<Vec2>(L, a - b);
  return 1;
}
static int lVec2Mul(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1);
  auto &b = udValue<Vec2>(L, 2);
  udNewOwned<Vec2>(L, a * b);
  return 1;
}
static int lVec2Div(lua_State *L) {
  auto &a = udValue<Vec2>(L, 1);
  auto &b = udValue<Vec2>(L, 2);
  udNewOwned<Vec2>(L, a / b);
  return 1;
}
static int lVec2Tostring(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1);
  String s = v.tableString();
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int lVec2Copy(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1);
  udNewOwned<Vec2>(L, v);
  return 1;
}
static int lVec2Length(lua_State *L) {
  auto &v = udValue<Vec2>(L, 1);
  lua_pushnumber(L, v.length());
  return 1;
}
void registerVec2(lua_State *L) {
  luaL_newmetatable(L, Vec2::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Vec2>);
  Binding::registerFunction(L, "__index", lVec2Index);
  Binding::registerFunction(L, "__newindex", lVec2Newindex);
  Binding::registerFunction(L, "__add", lVec2Add);
  Binding::registerFunction(L, "__sub", lVec2Sub);
  Binding::registerFunction(L, "__mul", lVec2Mul);
  Binding::registerFunction(L, "__div", lVec2Div);
  Binding::registerFunction(L, "__tostring", lVec2Tostring);
  Binding::registerFunction(L, "copy", lVec2Copy);
  Binding::registerFunction(L, "length", lVec2Length);
  lua_pop(L, 1);

  pushSnNamed(L, "Vec2");
  Binding::registerFunction(L, "new", lVec2New);
  lua_pop(L, 1);
}
} // namespace sinen
