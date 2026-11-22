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
static auto lWorld2DNew() { return World2D::create(); }

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

static auto lWorld2DRemoveCollider(Ptr<World2D> w, Collider &c) {
  w->removeCollider(c);
  return;
}

static auto lWorld2DDestroyCollider(Ptr<World2D> w, Collider &c) {
  w->destroyCollider(c);
  return;
}

static auto lWorld2DSetGravity(Ptr<World2D> w, Vec3 &g) {
  w->setGravity(g);
  return;
}

static auto lWorld2DGetGravity(Ptr<World2D> w) { return w->getGravity(); }

static auto lWorld2DBodyCount(Ptr<World2D> w) {
  return static_cast<lua_Integer>(w->bodyCount());
}

static auto lWorld2DOptimizeBroadPhase(Ptr<World2D> w) {
  w->optimizeBroadPhase();
  return;
}

static auto lWorld2DUpdate(Ptr<World2D> w, float time, int collisionSteps) {
  w->update(time, collisionSteps);
  return;
}

static auto lWorld3DNew() { return World3D::create(); }

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
  luau::table(L, -1).value("fraction", hit.fraction);
  luau::table(L, -1).value("distance", hit.distance);
  luau::table(L, -1).value("userMaterialId",
                           static_cast<lua_Integer>(hit.userMaterialId));
  luau::table(L, -1).value("triangleIndex",
                           static_cast<lua_Integer>(hit.triangleIndex));
  luau::table(L, -1).value("childIndex",
                           static_cast<lua_Integer>(hit.childIndex));
  return 1;
}
static auto lWorld3DRemoveCollider(Ptr<World3D> w, Collider &c) {
  w->removeCollider(c);
  return;
}
static auto lWorld3DDestroyCollider(Ptr<World3D> w, Collider &c) {
  w->destroyCollider(c);
  return;
}

static auto lWorld3DSetGravity(Ptr<World3D> w, Vec3 &g) {
  w->setGravity(g);
  return;
}
static auto lWorld3DGetGravity(Ptr<World3D> w) { return w->getGravity(); }
static auto lWorld3DBodyCount(Ptr<World3D> w) {
  return static_cast<lua_Integer>(w->bodyCount());
}
static auto lWorld3DOptimizeBroadPhase(Ptr<World3D> w) {
  w->optimizeBroadPhase();
  return;
}
static auto lWorld3DUpdate(Ptr<World3D> w, float time, int collisionSteps) {
  w->update(time, collisionSteps);
  return;
}
void registerPhysics(lua_State *L) {
  pushClassMetatable<Ptr<World2D>>(L, World2D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).raw_function("newBoxCollider", lWorld2DNewBoxCollider);
  luau::table(L, -1).raw_function("newCircleCollider",
                                  lWorld2DNewCircleCollider);
  luau::table(L, -1).raw_function("newCapsuleCollider",
                                  lWorld2DNewCapsuleCollider);
  luau::table(L, -1).raw_function("addCollider", lWorld2DAddCollider);
  luau::table(L, -1).function("removeCollider", lWorld2DRemoveCollider);
  luau::table(L, -1).function("destroyCollider", lWorld2DDestroyCollider);
  luau::table(L, -1).function("setGravity", lWorld2DSetGravity);
  luau::table(L, -1).function("getGravity", lWorld2DGetGravity);
  luau::table(L, -1).function("bodyCount", lWorld2DBodyCount);
  luau::table(L, -1).function("optimizeBroadPhase", lWorld2DOptimizeBroadPhase);
  luau::table(L, -1).function("update", lWorld2DUpdate);
  lua_pop(L, 1);

  pushClassMetatable<Ptr<World3D>>(L, World3D::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).raw_function("newBoxCollider", lWorld3DNewBoxCollider);
  luau::table(L, -1).raw_function("newSphereCollider",
                                  lWorld3DNewSphereCollider);
  luau::table(L, -1).raw_function("newCylinderCollider",
                                  lWorld3DNewCylinderCollider);
  luau::table(L, -1).raw_function("newCapsuleCollider",
                                  lWorld3DNewCapsuleCollider);
  luau::table(L, -1).raw_function("addCollider", lWorld3DAddCollider);
  luau::table(L, -1).raw_function("raycastClosest", lWorld3DRaycastClosest);
  luau::table(L, -1).function("removeCollider", lWorld3DRemoveCollider);
  luau::table(L, -1).function("destroyCollider", lWorld3DDestroyCollider);
  luau::table(L, -1).function("setGravity", lWorld3DSetGravity);
  luau::table(L, -1).function("getGravity", lWorld3DGetGravity);
  luau::table(L, -1).function("bodyCount", lWorld3DBodyCount);
  luau::table(L, -1).function("optimizeBroadPhase", lWorld3DOptimizeBroadPhase);
  luau::table(L, -1).function("update", lWorld3DUpdate);
  lua_pop(L, 1);

  pushSnNamed(L, "World2D");
  luau::table(L, -1).function("new", lWorld2DNew);
  lua_pop(L, 1);

  pushSnNamed(L, "World3D");
  luau::table(L, -1).function("new", lWorld3DNew);
  lua_pop(L, 1);
}
} // namespace sinen
