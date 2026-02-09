#include <physics/collider.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lColliderGetPosition(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getPosition());
  return 1;
}
static int lColliderGetVelocity(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getVelocity());
  return 1;
}
static int lColliderSetLinearVelocity(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &v = udValue<Vec3>(L, 2);
  c.setLinearVelocity(v);
  return 0;
}
void registerCollider(lua_State *L) {
  luaL_newmetatable(L, Collider::metaTableName());
  luaPushcfunction2(L, udGc<Collider>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lColliderGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lColliderGetVelocity);
  lua_setfield(L, -2, "getVelocity");
  luaPushcfunction2(L, lColliderSetLinearVelocity);
  lua_setfield(L, -2, "setLinearVelocity");
  lua_pop(L, 1);

  pushSnNamed(L, "Collider");
  lua_pop(L, 2);
}
} // namespace sinen
