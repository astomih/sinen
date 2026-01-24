#include "transform.hpp"
#include <math/matrix.hpp>
#include <math/quaternion.hpp>

namespace sinen {
Mat4 Transform::getWorldMatrix() const {
  const auto t = Mat4::translate(position);
  const auto r = Mat4::fromQuat(Quat::fromEuler(rotation));
  const auto s = Mat4::scale(scale);
  return t * r * s;
}
} // namespace sinen

#include <asset/script/luaapi.hpp>
namespace sinen {
String Transform::tableString() const {
  TablePair p;
  p.emplace_back("position", position.tableString());
  p.emplace_back("rotation", rotation.tableString());
  p.emplace_back("scale", scale.tableString());
  return convert("sn.Transform", p, true);
}
static int lTransformNew(lua_State *L) {
  udNewOwned<Transform>(L, Transform());
  return 1;
}
static int lTransformIndex(lua_State *L) {
  auto &t = udValue<Transform>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "position") == 0) {
    udNewRef<Vec3>(L, &t.position);
    return 1;
  }
  if (std::strcmp(k, "rotation") == 0) {
    udNewRef<Vec3>(L, &t.rotation);
    return 1;
  }
  if (std::strcmp(k, "scale") == 0) {
    udNewRef<Vec3>(L, &t.scale);
    return 1;
  }
  luaL_getmetatable(L, Transform::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lTransformNewindex(lua_State *L) {
  auto &t = udValue<Transform>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3);
  if (std::strcmp(k, "position") == 0) {
    t.position = v;
    return 0;
  }
  if (std::strcmp(k, "rotation") == 0) {
    t.rotation = v;
    return 0;
  }
  if (std::strcmp(k, "scale") == 0) {
    t.scale = v;
    return 0;
  }
  return luaLError2(L, "sn.Transform: invalid field '%s'", k);
}
static int lTransformTostring(lua_State *L) {
  auto &t = udValue<Transform>(L, 1);
  String s = t.tableString();
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
void registerTransform(lua_State *L) {
  luaL_newmetatable(L, Transform::metaTableName());
  luaPushcfunction2(L, udGc<Transform>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lTransformIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTransformNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lTransformTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Transform");
  luaPushcfunction2(L, lTransformNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen