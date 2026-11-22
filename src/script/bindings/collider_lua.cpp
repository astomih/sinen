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
void registerCollider(lua_State *L) {
  pushClassMetatable<Collider>(L, Collider::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).raw_function("isValid", lColliderIsValid);
  luau::table(L, -1).raw_function("isAdded", lColliderIsAdded);
  luau::table(L, -1).raw_function("getPosition", lColliderGetPosition);
  luau::table(L, -1).raw_function("getRotation", lColliderGetRotation);
  luau::table(L, -1).raw_function("getVelocity", lColliderGetVelocity);
  luau::table(L, -1).raw_function("getAngularVelocity",
                                  lColliderGetAngularVelocity);
  luau::table(L, -1).raw_function("setPosition", lColliderSetPosition);
  luau::table(L, -1).raw_function("setRotation", lColliderSetRotation);
  luau::table(L, -1).raw_function("setPositionAndRotation",
                                  lColliderSetPositionAndRotation);
  luau::table(L, -1).function("setLinearVelocity", lColliderSetLinearVelocity);
  luau::table(L, -1).function("setAngularVelocity",
                              lColliderSetAngularVelocity);
  luau::table(L, -1).raw_function("addForce", lColliderAddForce);
  luau::table(L, -1).function("addImpulse", lColliderAddImpulse);
  luau::table(L, -1).function("setFriction", lColliderSetFriction);
  luau::table(L, -1).function("setRestitution", lColliderSetRestitution);
  luau::table(L, -1).raw_function("activate", lColliderActivate);
  luau::table(L, -1).raw_function("deactivate", lColliderDeactivate);
  luau::table(L, -1).raw_function("remove", lColliderRemove);
  luau::table(L, -1).raw_function("destroy", lColliderDestroy);
  lua_pop(L, 1);
}
} // namespace sinen
