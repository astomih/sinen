#include "luaapi.hpp"
#include <math/geometry/bbox.hpp>

namespace sinen {
static auto lAabbNew() { return AABB{}; }
static int lAabbIndex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "min") == 0) {
    udNewRef<Vec3>(L, &aabb.min);
    return 1;
  }
  if (std::strcmp(k, "max") == 0) {
    udNewRef<Vec3>(L, &aabb.max);
    return 1;
  }
  luaL_getmetatable(L, AABB::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lAabbNewindex(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  auto &v = udValue<Vec3>(L, 3);
  if (std::strcmp(k, "min") == 0) {
    aabb.min = v;
    return 0;
  }
  if (std::strcmp(k, "max") == 0) {
    aabb.max = v;
    return 0;
  }
  return luaLError2(L, "sn.AABB: invalid field '%s'", k);
}
static auto lAabbUpdateWorld(AABB &aabb, Vec3 &p, Vec3 &scale, AABB &local) {
  aabb.updateWorld(p, scale, local);
  return;
}
static auto lAabbIntersectsAabb(AABB &a, AABB &b) {
  return a.intersectsAABB(b);
}
void registerAABB(lua_State *L) {
  pushClassMetatable<AABB>(L, AABB::metaTableName());
  luau::table(L, -1).raw_function("__index", lAabbIndex);
  luau::table(L, -1).raw_function("__newindex", lAabbNewindex);
  luau::table(L, -1).function("updateWorld", lAabbUpdateWorld);
  luau::table(L, -1).function("intersectsAABB", lAabbIntersectsAabb);
  lua_pop(L, 1);

  pushSnNamed(L, "AABB");
  luau::table(L, -1).function("new", lAabbNew);
  lua_pop(L, 1);
}
} // namespace sinen
