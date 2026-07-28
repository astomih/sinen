#include "luaapi.hpp"
#include <math/geometry/ray.hpp>

namespace sinen {
static int lRayIndex(lua_State *L) {
  auto &r = udValue<Ray>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "origin") == 0) {
    udNewRef<Vec3>(L, &r.origin);
    return 1;
  }
  if (std::strcmp(k, "direction") == 0) {
    udNewRef<Vec3>(L, &r.direction);
    return 1;
  }
  luaL_getmetatable(L, Ray::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lRayNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned(L, Ray{});
    return 1;
  }
  auto origin = udValue<Vec3>(L, 1);
  auto direction = udValue<Vec3>(L, 2);
  udNewOwned(L, Ray{origin, direction});
  return 1;
}
void registerRay(lua_State *L) {
  luaL_newmetatable(L, Ray::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<Ray>);
  Binding::registerFunction(L, "__index", lRayIndex);
  lua_pop(L, 1);

  pushSnNamed(L, "Ray");
  Binding::registerFunction(L, "new", lRayNew);
  lua_pop(L, 1);
}
} // namespace sinen
