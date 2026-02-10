#include <physics/collider.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lColliderIsValid(lua_State *L) {
  lua_pushboolean(L, udValue<Collider>(L, 1).isValid() ? 1 : 0);
  return 1;
}
static int lColliderIsAdded(lua_State *L) {
  lua_pushboolean(L, udValue<Collider>(L, 1).isAdded() ? 1 : 0);
  return 1;
}
static int lColliderGetPosition(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getPosition());
  return 1;
}
static int lColliderGetRotation(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getRotation());
  return 1;
}
static int lColliderGetVelocity(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getVelocity());
  return 1;
}
static int lColliderGetAngularVelocity(lua_State *L) {
  udNewOwned<Vec3>(L, udValue<Collider>(L, 1).getAngularVelocity());
  return 1;
}
static int lColliderSetPosition(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &p = udValue<Vec3>(L, 2);
  bool activate = lua_gettop(L) >= 3 ? (lua_toboolean(L, 3) != 0) : true;
  c.setPosition(p, activate);
  return 0;
}
static int lColliderSetRotation(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &r = udValue<Vec3>(L, 2);
  bool activate = lua_gettop(L) >= 3 ? (lua_toboolean(L, 3) != 0) : true;
  c.setRotation(r, activate);
  return 0;
}
static int lColliderSetPositionAndRotation(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &p = udValue<Vec3>(L, 2);
  auto &r = udValue<Vec3>(L, 3);
  bool activate = lua_gettop(L) >= 4 ? (lua_toboolean(L, 4) != 0) : true;
  c.setPositionAndRotation(p, r, activate);
  return 0;
}
static int lColliderSetLinearVelocity(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &v = udValue<Vec3>(L, 2);
  c.setLinearVelocity(v);
  return 0;
}
static int lColliderSetAngularVelocity(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &v = udValue<Vec3>(L, 2);
  c.setAngularVelocity(v);
  return 0;
}
static int lColliderAddForce(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &f = udValue<Vec3>(L, 2);
  bool activate = lua_gettop(L) >= 3 ? (lua_toboolean(L, 3) != 0) : true;
  c.addForce(f, activate);
  return 0;
}
static int lColliderAddImpulse(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &i = udValue<Vec3>(L, 2);
  c.addImpulse(i);
  return 0;
}
static int lColliderSetFriction(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  c.setFriction(v);
  return 0;
}
static int lColliderSetRestitution(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  float v = static_cast<float>(luaL_checknumber(L, 2));
  c.setRestitution(v);
  return 0;
}
static int lColliderActivate(lua_State *L) {
  udValue<Collider>(L, 1).activate();
  return 0;
}
static int lColliderDeactivate(lua_State *L) {
  udValue<Collider>(L, 1).deactivate();
  return 0;
}
static int lColliderRemove(lua_State *L) {
  udValue<Collider>(L, 1).remove();
  return 0;
}
static int lColliderDestroy(lua_State *L) {
  udValue<Collider>(L, 1).destroy();
  return 0;
}
void registerCollider(lua_State *L) {
  luaL_newmetatable(L, Collider::metaTableName());
  luaPushcfunction2(L, udGc<Collider>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lColliderIsValid);
  lua_setfield(L, -2, "isValid");
  luaPushcfunction2(L, lColliderIsAdded);
  lua_setfield(L, -2, "isAdded");
  luaPushcfunction2(L, lColliderGetPosition);
  lua_setfield(L, -2, "getPosition");
  luaPushcfunction2(L, lColliderGetRotation);
  lua_setfield(L, -2, "getRotation");
  luaPushcfunction2(L, lColliderGetVelocity);
  lua_setfield(L, -2, "getVelocity");
  luaPushcfunction2(L, lColliderGetAngularVelocity);
  lua_setfield(L, -2, "getAngularVelocity");
  luaPushcfunction2(L, lColliderSetPosition);
  lua_setfield(L, -2, "setPosition");
  luaPushcfunction2(L, lColliderSetRotation);
  lua_setfield(L, -2, "setRotation");
  luaPushcfunction2(L, lColliderSetPositionAndRotation);
  lua_setfield(L, -2, "setPositionAndRotation");
  luaPushcfunction2(L, lColliderSetLinearVelocity);
  lua_setfield(L, -2, "setLinearVelocity");
  luaPushcfunction2(L, lColliderSetAngularVelocity);
  lua_setfield(L, -2, "setAngularVelocity");
  luaPushcfunction2(L, lColliderAddForce);
  lua_setfield(L, -2, "addForce");
  luaPushcfunction2(L, lColliderAddImpulse);
  lua_setfield(L, -2, "addImpulse");
  luaPushcfunction2(L, lColliderSetFriction);
  lua_setfield(L, -2, "setFriction");
  luaPushcfunction2(L, lColliderSetRestitution);
  lua_setfield(L, -2, "setRestitution");
  luaPushcfunction2(L, lColliderActivate);
  lua_setfield(L, -2, "activate");
  luaPushcfunction2(L, lColliderDeactivate);
  lua_setfield(L, -2, "deactivate");
  luaPushcfunction2(L, lColliderRemove);
  lua_setfield(L, -2, "remove");
  luaPushcfunction2(L, lColliderDestroy);
  lua_setfield(L, -2, "destroy");
  lua_pop(L, 1);

  pushSnNamed(L, "Collider");
  lua_pop(L, 2);
}
} // namespace sinen
