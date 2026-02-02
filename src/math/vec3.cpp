#include "vec3.hpp"
#include "mat4.hpp"
#include "vec4.hpp"

namespace sinen {
Vec3::Vec3(const Vec4 &v) : x(v.x), y(v.y), z(v.z) {}

Vec3 Vec3::transform(const Vec3 &vec, const Mat4 &mat, float w /*= 1.0f*/) {
  Vec3 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
             vec.z * mat.mat[2][0] + w * mat.mat[3][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
             vec.z * mat.mat[2][1] + w * mat.mat[3][1];
  retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
             vec.z * mat.mat[2][2] + w * mat.mat[3][2];
  // ignore w since we aren't returning a new value for it...
  return retVal;
}

// This will transform the vector and renormalize the w component
Vec3 Vec3::transformWithPerspDiv(const Vec3 &vec, const Mat4 &mat,
                                 float w /*= 1.0f*/) {
  Vec3 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
             vec.z * mat.mat[2][0] + w * mat.mat[3][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
             vec.z * mat.mat[2][1] + w * mat.mat[3][1];
  retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
             vec.z * mat.mat[2][2] + w * mat.mat[3][2];
  float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
                       vec.z * mat.mat[2][3] + w * mat.mat[3][3];
  if (!Math::nearZero(Math::abs(transformedW))) {
    transformedW = 1.0f / transformedW;
    retVal *= transformedW;
  }
  return retVal;
}

// Transform a Vector3 by a quaternion
Vec3 Vec3::transform(const Vec3 &v, const Quat &q) {
  // v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
  Vec3 qv(q.x, q.y, q.z);
  Vec3 retVal = v;
  retVal += 2.0f * Vec3::cross(qv, Vec3::cross(qv, v) + q.w * v);
  return retVal;
}
} // namespace sinen

#include <script/luaapi.hpp>
namespace sinen {
String Vec3::tableString() const {
  TablePair p;
  p.emplace_back("x", toStringTrim(x));
  p.emplace_back("y", toStringTrim(y));
  p.emplace_back("z", toStringTrim(z));
  return convert("sn.Vec3", p, false);
}
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
  luaPushcfunction2(L, udGc<Vec3>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lVec3Index);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lVec3Newindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lVec3Add);
  lua_setfield(L, -2, "__add");
  luaPushcfunction2(L, lVec3Sub);
  lua_setfield(L, -2, "__sub");
  luaPushcfunction2(L, lVec3Mul);
  lua_setfield(L, -2, "__mul");
  luaPushcfunction2(L, lVec3Div);
  lua_setfield(L, -2, "__div");
  luaPushcfunction2(L, lVec3Tostring);
  lua_setfield(L, -2, "__tostring");
  luaPushcfunction2(L, lVec3Copy);
  lua_setfield(L, -2, "copy");
  luaPushcfunction2(L, lVec3Length);
  lua_setfield(L, -2, "length");
  luaPushcfunction2(L, lVec3Normalize);
  lua_setfield(L, -2, "normalize");
  luaPushcfunction2(L, lVec3Dot);
  lua_setfield(L, -2, "dot");
  luaPushcfunction2(L, lVec3Cross);
  lua_setfield(L, -2, "cross");
  luaPushcfunction2(L, lVec3Lerp);
  lua_setfield(L, -2, "lerp");
  luaPushcfunction2(L, lVec3Reflect);
  lua_setfield(L, -2, "reflect");
  lua_pop(L, 1);

  pushSnNamed(L, "Vec3");
  luaPushcfunction2(L, lVec3New);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen