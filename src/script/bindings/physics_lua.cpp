#include "luaapi.hpp"
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <core/time/time.hpp>
#include <math/geometry/ray.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
#include <physics/physics.hpp>
#include <physics/world2d.hpp>
#include <physics/world3d.hpp>

namespace sinen {
static int lWorld2DNew(lua_State *L) {
  udPushPtr<World2D>(L, World2D::create());
  return 1;
}

static int lWorld2DNewBoxCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &t = udValue<Transform>(L, 2);
  bool isStatic = lua_toboolean(L, 3) != 0;
  udNewOwned<Collider>(L, w->newBoxCollider(t, isStatic));
  return 1;
}

static int lWorld2DNewCircleCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  float radius = static_cast<float>(luaL_checknumber(L, 3));
  bool isStatic = lua_toboolean(L, 4) != 0;
  udNewOwned<Collider>(L, w->newCircleCollider(pos, radius, isStatic));
  return 1;
}

static int lWorld2DNewCapsuleCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  auto &rot = udValue<Vec3>(L, 3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 4));
  float radius = static_cast<float>(luaL_checknumber(L, 5));
  bool isStatic = lua_toboolean(L, 6) != 0;
  udNewOwned<Collider>(
      L, w->newCapsuleCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}

static int lWorld2DAddCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  bool active = lua_toboolean(L, 3) != 0;
  w->addCollider(c, active);
  return 0;
}

static int lWorld2DRemoveCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  w->removeCollider(c);
  return 0;
}

static int lWorld2DDestroyCollider(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  w->destroyCollider(c);
  return 0;
}

static int lWorld2DSetGravity(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  auto &g = udValue<Vec3>(L, 2);
  w->setGravity(g);
  return 0;
}

static int lWorld2DGetGravity(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  udNewOwned<Vec3>(L, w->getGravity());
  return 1;
}

static int lWorld2DBodyCount(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  lua_pushinteger(L, static_cast<lua_Integer>(w->bodyCount()));
  return 1;
}

static int lWorld2DOptimizeBroadPhase(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  w->optimizeBroadPhase();
  return 0;
}

static int lWorld2DUpdate(lua_State *L) {
  auto w = udPtr<World2D>(L, 1);
  float time = static_cast<float>(luaL_checknumber(L, 2));
  int collisionSteps = static_cast<int>(luaL_checkinteger(L, 3));
  w->update(time, collisionSteps);
  return 0;
}

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
static int lWorld3DRaycastClosest(lua_State *L) {
  auto w = udPtr<World3D>(L, 1);
  Vec3 origin;
  Vec3 direction;
  float maxDistance = 0.0f;

  if (udValueOrNull<Ray>(L, 2) != nullptr) {
    const auto &ray = udValue<Ray>(L, 2);
    origin = ray.origin;
    direction = ray.direction;
    maxDistance = lua_gettop(L) >= 3
                      ? static_cast<float>(luaL_checknumber(L, 3))
                      : direction.length();
  } else {
    origin = udValue<Vec3>(L, 2);
    direction = udValue<Vec3>(L, 3);
    maxDistance = lua_gettop(L) >= 4
                      ? static_cast<float>(luaL_checknumber(L, 4))
                      : direction.length();
  }

  RaycastHit3D hit;
  if (!w->raycastClosest(origin, direction, maxDistance, hit)) {
    lua_pushnil(L);
    return 1;
  }

  lua_createtable(L, 0, 8);
  udNewOwned<Collider>(L, Collider(*w, hit.colliderId));
  lua_setfield(L, -2, "collider");
  udNewOwned<Vec3>(L, hit.point);
  lua_setfield(L, -2, "point");
  udNewOwned<Vec3>(L, hit.normal);
  lua_setfield(L, -2, "normal");
  Binding::registerNumber(L, "fraction", hit.fraction);
  Binding::registerNumber(L, "distance", hit.distance);
  Binding::registerInteger(L, "userMaterialId",
                           static_cast<lua_Integer>(hit.userMaterialId));
  Binding::registerInteger(L, "triangleIndex",
                           static_cast<lua_Integer>(hit.triangleIndex));
  Binding::registerInteger(L, "childIndex",
                           static_cast<lua_Integer>(hit.childIndex));
  return 1;
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
  luaL_newmetatable(L, World2D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "newBoxCollider", lWorld2DNewBoxCollider);
  Binding::registerFunction(L, "newCircleCollider", lWorld2DNewCircleCollider);
  Binding::registerFunction(L, "newCapsuleCollider",
                            lWorld2DNewCapsuleCollider);
  Binding::registerFunction(L, "addCollider", lWorld2DAddCollider);
  Binding::registerFunction(L, "removeCollider", lWorld2DRemoveCollider);
  Binding::registerFunction(L, "destroyCollider", lWorld2DDestroyCollider);
  Binding::registerFunction(L, "setGravity", lWorld2DSetGravity);
  Binding::registerFunction(L, "getGravity", lWorld2DGetGravity);
  Binding::registerFunction(L, "bodyCount", lWorld2DBodyCount);
  Binding::registerFunction(L, "optimizeBroadPhase",
                            lWorld2DOptimizeBroadPhase);
  Binding::registerFunction(L, "update", lWorld2DUpdate);
  lua_pop(L, 1);

  luaL_newmetatable(L, World3D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "newBoxCollider", lWorld3DNewBoxCollider);
  Binding::registerFunction(L, "newSphereCollider", lWorld3DNewSphereCollider);
  Binding::registerFunction(L, "newCylinderCollider",
                            lWorld3DNewCylinderCollider);
  Binding::registerFunction(L, "newCapsuleCollider",
                            lWorld3DNewCapsuleCollider);
  Binding::registerFunction(L, "addCollider", lWorld3DAddCollider);
  Binding::registerFunction(L, "raycastClosest", lWorld3DRaycastClosest);
  Binding::registerFunction(L, "removeCollider", lWorld3DRemoveCollider);
  Binding::registerFunction(L, "destroyCollider", lWorld3DDestroyCollider);
  Binding::registerFunction(L, "setGravity", lWorld3DSetGravity);
  Binding::registerFunction(L, "getGravity", lWorld3DGetGravity);
  Binding::registerFunction(L, "bodyCount", lWorld3DBodyCount);
  Binding::registerFunction(L, "optimizeBroadPhase",
                            lWorld3DOptimizeBroadPhase);
  Binding::registerFunction(L, "update", lWorld3DUpdate);
  lua_pop(L, 1);

  pushSnNamed(L, "World2D");
  Binding::registerFunction(L, "new", lWorld2DNew);
  lua_pop(L, 1);

  pushSnNamed(L, "World3D");
  Binding::registerFunction(L, "new", lWorld3DNew);
  lua_pop(L, 1);
}
} // namespace sinen
