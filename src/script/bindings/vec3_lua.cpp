#include "luaapi.hpp"
#include <math/quaternion.hpp>
#include <math/vec3.hpp>

namespace sinen {
static int lVec3New(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Vec3>(L, Vec3(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Vec3>(L, Vec3(v));
    return 1;
  }
  float x = static_cast<float>(luaL_checknumber(L, 1));
  float y = static_cast<float>(luaL_checknumber(L, 2));
  float z = static_cast<float>(luaL_checknumber(L, 3));
  udNewOwned<Vec3>(L, Vec3(x, y, z));
  return 1;
}
static int lVec3Index(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, v.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, v.y);
    return 1;
  }
  if (std::strcmp(k, "z") == 0) {
    lua_pushnumber(L, v.z);
    return 1;
  }
  luaL_getmetatable(L, Vec3::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lVec3Newindex(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1);
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
  if (std::strcmp(k, "z") == 0) {
    v.z = value;
    return 0;
  }
  return luaLError2(L, "sn.Vec3: invalid field '%s'", k);
}
static int lVec3Add(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1);
  auto &b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, a + b);
  return 1;
}
static int lVec3Sub(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1);
  auto &b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, a - b);
  return 1;
}
static int lVec3Mul(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1);
  auto &b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, a * b);
  return 1;
}
static int lVec3Div(lua_State *L) {
  auto &a = udValue<Vec3>(L, 1);
  auto &b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, a / b);
  return 1;
}
static int lVec3Tostring(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1);
  String s = v.tableString();
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
static int lVec3Copy(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1);
  udNewOwned<Vec3>(L, v);
  return 1;
}
static int lVec3Length(lua_State *L) {
  auto &v = udValue<Vec3>(L, 1);
  lua_pushnumber(L, v.length());
  return 1;
}
static int lVec3Forward(lua_State *L) {
  auto v = udValue<Vec3>(L, 1);
  auto &rotation = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, Vec3::transform(v, Quat::fromEuler(rotation)));
  return 1;
}
static int lVec3Normalize(lua_State *L) {
  auto v = udValue<Vec3>(L, 1);
  v.normalize();
  udNewOwned<Vec3>(L, v);
  return 1;
}
static int lVec3Dot(lua_State *L) {
  auto a = udValue<Vec3>(L, 1);
  auto b = udValue<Vec3>(L, 2);
  lua_pushnumber(L, Vec3::dot(a, b));
  return 1;
}
static int lVec3Cross(lua_State *L) {
  auto a = udValue<Vec3>(L, 1);
  auto b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, Vec3::cross(a, b));
  return 1;
}
static int lVec3Lerp(lua_State *L) {
  auto a = udValue<Vec3>(L, 1);
  auto b = udValue<Vec3>(L, 2);
  float value = luaL_checknumber(L, 3);
  udNewOwned<Vec3>(L, Vec3::lerp(a, b, value));
  return 1;
}
static int lVec3Reflect(lua_State *L) {
  auto a = udValue<Vec3>(L, 1);
  auto b = udValue<Vec3>(L, 2);
  udNewOwned<Vec3>(L, Vec3::reflect(a, b));
  return 1;
}
void registerVec3(lua_State *L) {
  luaL_newmetatable(L, Vec3::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Vec3>);
  Binding::registerFunction(L, "__index", lVec3Index);
  Binding::registerFunction(L, "__newindex", lVec3Newindex);
  Binding::registerFunction(L, "__add", lVec3Add);
  Binding::registerFunction(L, "__sub", lVec3Sub);
  Binding::registerFunction(L, "__mul", lVec3Mul);
  Binding::registerFunction(L, "__div", lVec3Div);
  Binding::registerFunction(L, "__tostring", lVec3Tostring);
  Binding::registerFunction(L, "copy", lVec3Copy);
  Binding::registerFunction(L, "length", lVec3Length);
  Binding::registerFunction(L, "forward", lVec3Forward);
  Binding::registerFunction(L, "normalize", lVec3Normalize);
  Binding::registerFunction(L, "dot", lVec3Dot);
  Binding::registerFunction(L, "cross", lVec3Cross);
  Binding::registerFunction(L, "lerp", lVec3Lerp);
  Binding::registerFunction(L, "reflect", lVec3Reflect);
  lua_pop(L, 1);

  pushSnNamed(L, "Vec3");
  Binding::registerFunction(L, "new", lVec3New);
  lua_pop(L, 1);
}
} // namespace sinen
