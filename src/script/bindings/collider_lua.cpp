#include "luaapi.hpp"
#include <physics/collider.hpp>

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
  Binding::registerFunction(L, "__gc", udGc<Collider>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "isValid", lColliderIsValid);
  Binding::registerFunction(L, "isAdded", lColliderIsAdded);
  Binding::registerFunction(L, "getPosition", lColliderGetPosition);
  Binding::registerFunction(L, "getRotation", lColliderGetRotation);
  Binding::registerFunction(L, "getVelocity", lColliderGetVelocity);
  Binding::registerFunction(L, "getAngularVelocity",
                            lColliderGetAngularVelocity);
  Binding::registerFunction(L, "setPosition", lColliderSetPosition);
  Binding::registerFunction(L, "setRotation", lColliderSetRotation);
  Binding::registerFunction(L, "setPositionAndRotation",
                            lColliderSetPositionAndRotation);
  Binding::registerFunction(L, "setLinearVelocity", lColliderSetLinearVelocity);
  Binding::registerFunction(L, "setAngularVelocity",
                            lColliderSetAngularVelocity);
  Binding::registerFunction(L, "addForce", lColliderAddForce);
  Binding::registerFunction(L, "addImpulse", lColliderAddImpulse);
  Binding::registerFunction(L, "setFriction", lColliderSetFriction);
  Binding::registerFunction(L, "setRestitution", lColliderSetRestitution);
  Binding::registerFunction(L, "activate", lColliderActivate);
  Binding::registerFunction(L, "deactivate", lColliderDeactivate);
  Binding::registerFunction(L, "remove", lColliderRemove);
  Binding::registerFunction(L, "destroy", lColliderDestroy);
  lua_pop(L, 1);
}
} // namespace sinen
