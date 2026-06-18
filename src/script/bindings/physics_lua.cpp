#include "luaapi.hpp"
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <core/time/time.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
#include <physics/physics.hpp>
#include <physics/world3d.hpp>

namespace sinen {
static int lWorld3DNew(lua_State *L) {
  udPushPtr<World3D>(L, World3D::create());
  return 1;
}

static int lWorld3DNewBoxCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &t = udValue<Transform>(L, 2);
  bool isStatic = lua_toboolean(L, 3) != 0;
  auto collider = w->newBoxCollider(t, isStatic);
  udNewOwned<Collider>(L, collider);
  return 1;
}
static int lWorld3DNewSphereCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  float radius = static_cast<float>(luaL_checknumber(L, 3));
  bool isStatic = lua_toboolean(L, 4) != 0;
  udNewOwned<Collider>(L, w->newSphereCollider(pos, radius, isStatic));
  return 1;
}
static int lWorld3DNewCylinderCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  auto &rot = udValue<Vec3>(L, 3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 4));
  float radius = static_cast<float>(luaL_checknumber(L, 5));
  bool isStatic = lua_toboolean(L, 6) != 0;
  udNewOwned<Collider>(
      L, w->newCylinderCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}
static int lWorld3DNewCapsuleCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  auto &rot = udValue<Vec3>(L, 3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 4));
  float radius = static_cast<float>(luaL_checknumber(L, 5));
  bool isStatic = lua_toboolean(L, 6) != 0;
  udNewOwned<Collider>(
      L, w->newCapsuleCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}
static int lWorld3DAddCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  bool active = lua_toboolean(L, 3) != 0;
  w->addCollider(c, active);
  return 0;
}
static int lWorld3DRemoveCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  w->removeCollider(c);
  return 0;
}
static int lWorld3DDestroyCollider(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  w->destroyCollider(c);
  return 0;
}

static int lWorld3DSetGravity(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  auto &g = udValue<Vec3>(L, 2);
  w->setGravity(g);
  return 0;
}
static int lWorld3DGetGravity(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  udNewOwned<Vec3>(L, w->getGravity());
  return 1;
}
static int lWorld3DBodyCount(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  lua_pushinteger(L, static_cast<lua_Integer>(w->bodyCount()));
  return 1;
}
static int lWorld3DOptimizeBroadPhase(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  w->optimizeBroadPhase();
  return 0;
}
static int lWorld3DUpdate(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  float time = static_cast<float>(luaL_checknumber(L, 2));
  int collisionSteps = static_cast<int>(luaL_checkinteger(L, 3));
  w->update(time, collisionSteps);
  return 0;
}
void registerPhysics(lua_State *L) {
  luaL_newmetatable(L, World3D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lWorld3DNewBoxCollider);
  lua_setfield(L, -2, "newBoxCollider");
  luaPushcfunction2(L, lWorld3DNewSphereCollider);
  lua_setfield(L, -2, "newSphereCollider");
  luaPushcfunction2(L, lWorld3DNewCylinderCollider);
  lua_setfield(L, -2, "newCylinderCollider");
  luaPushcfunction2(L, lWorld3DNewCapsuleCollider);
  lua_setfield(L, -2, "newCapsuleCollider");
  luaPushcfunction2(L, lWorld3DAddCollider);
  lua_setfield(L, -2, "addCollider");
  luaPushcfunction2(L, lWorld3DRemoveCollider);
  lua_setfield(L, -2, "removeCollider");
  luaPushcfunction2(L, lWorld3DDestroyCollider);
  lua_setfield(L, -2, "destroyCollider");
  luaPushcfunction2(L, lWorld3DSetGravity);
  lua_setfield(L, -2, "setGravity");
  luaPushcfunction2(L, lWorld3DGetGravity);
  lua_setfield(L, -2, "getGravity");
  luaPushcfunction2(L, lWorld3DBodyCount);
  lua_setfield(L, -2, "bodyCount");
  luaPushcfunction2(L, lWorld3DOptimizeBroadPhase);
  lua_setfield(L, -2, "optimizeBroadPhase");
  luaPushcfunction2(L, lWorld3DUpdate);
  lua_setfield(L, -2, "update");
  lua_pop(L, 1);

  pushSnNamed(L, "World3D");
  luaPushcfunction2(L, lWorld3DNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
