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
static auto lColliderSetLinearVelocity(Collider &c, Vec3 &v) {
  c.setLinearVelocity(v);
  return;
}
static auto lColliderSetAngularVelocity(Collider &c, Vec3 &v) {
  c.setAngularVelocity(v);
  return;
}
static int lColliderAddForce(lua_State *L) {
  auto &c = udValue<Collider>(L, 1);
  auto &f = udValue<Vec3>(L, 2);
  bool activate = lua_gettop(L) >= 3 ? (lua_toboolean(L, 3) != 0) : true;
  c.addForce(f, activate);
  return 0;
}
static auto lColliderAddImpulse(Collider &c, Vec3 &i) {
  c.addImpulse(i);
  return;
}
static auto lColliderSetFriction(Collider &c, float v) {
  c.setFriction(v);
  return;
}
static auto lColliderSetRestitution(Collider &c, float v) {
  c.setRestitution(v);
  return;
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
void registerCollider(luau::table &sn) {
  auto collider = engineClass<Collider>(sn, Collider::metaTableName());

  collider.raw_method<lColliderIsValid>("isValid");
  collider.raw_method<lColliderIsAdded>("isAdded");
  collider.raw_method<lColliderGetPosition>("getPosition");
  collider.raw_method<lColliderGetRotation>("getRotation");
  collider.raw_method<lColliderGetVelocity>("getVelocity");
  collider.raw_method<lColliderGetAngularVelocity>("getAngularVelocity");
  collider.raw_method<lColliderSetPosition>("setPosition");
  collider.raw_method<lColliderSetRotation>("setRotation");
  collider.raw_method<lColliderSetPositionAndRotation>(
      "setPositionAndRotation");
  collider.method<lColliderSetLinearVelocity>("setLinearVelocity");
  collider.method<lColliderSetAngularVelocity>("setAngularVelocity");
  collider.raw_method<lColliderAddForce>("addForce");
  collider.method<lColliderAddImpulse>("addImpulse");
  collider.method<lColliderSetFriction>("setFriction");
  collider.method<lColliderSetRestitution>("setRestitution");
  collider.raw_method<lColliderActivate>("activate");
  collider.raw_method<lColliderDeactivate>("deactivate");
  collider.raw_method<lColliderRemove>("remove");
  collider.raw_method<lColliderDestroy>("destroy");
}
} // namespace sinen
