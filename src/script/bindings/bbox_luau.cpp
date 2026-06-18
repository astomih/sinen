#include "luaapi.hpp"
#include <math/geometry/bbox.hpp>

namespace sinen {
static int lAabbNew(lua_State *L) {
  udNewOwned<AABB>(L, AABB{});
  return 1;
}
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
static int lAabbUpdateWorld(lua_State *L) {
  auto &aabb = udValue<AABB>(L, 1);
  auto &p = udValue<Vec3>(L, 2);
  auto &scale = udValue<Vec3>(L, 3);
  auto &local = udValue<AABB>(L, 4);
  aabb.updateWorld(p, scale, local);
  return 0;
}
static int lAabbIntersectsAabb(lua_State *L) {
  auto &a = udValue<AABB>(L, 1);
  auto &b = udValue<AABB>(L, 2);
  lua_pushboolean(L, a.intersectsAABB(b));
  return 1;
}
void registerAABB(lua_State *L) {
  luaL_newmetatable(L, AABB::metaTableName());
  luaPushcfunction2(L, udGc<AABB>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lAabbIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lAabbNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lAabbUpdateWorld);
  lua_setfield(L, -2, "updateWorld");
  luaPushcfunction2(L, lAabbIntersectsAabb);
  lua_setfield(L, -2, "intersectsAABB");
  lua_pop(L, 1);

  pushSnNamed(L, "AABB");
  luaPushcfunction2(L, lAabbNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
