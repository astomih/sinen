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
void registerPhysics(luau::table &sn) {
  auto world2D = engineClass<Ptr<World2D>>(sn, World2D::metaTableName());

  world2D.raw_method<lWorld2DNewBoxCollider>("newBoxCollider");
  world2D.raw_method<lWorld2DNewCircleCollider>("newCircleCollider");
  world2D.raw_method<lWorld2DNewCapsuleCollider>("newCapsuleCollider");
  world2D.raw_method<lWorld2DAddCollider>("addCollider");
  world2D.method<lWorld2DRemoveCollider>("removeCollider");
  world2D.method<lWorld2DDestroyCollider>("destroyCollider");
  world2D.method<lWorld2DSetGravity>("setGravity");
  world2D.method<lWorld2DGetGravity>("getGravity");
  world2D.method<lWorld2DBodyCount>("bodyCount");
  world2D.method<lWorld2DOptimizeBroadPhase>("optimizeBroadPhase");
  world2D.method<lWorld2DUpdate>("update");

  auto world3D = engineClass<Ptr<World3D>>(sn, World3D::metaTableName());

  world3D.raw_method<lWorld3DNewBoxCollider>("newBoxCollider");
  world3D.raw_method<lWorld3DNewSphereCollider>("newSphereCollider");
  world3D.raw_method<lWorld3DNewCylinderCollider>("newCylinderCollider");
  world3D.raw_method<lWorld3DNewCapsuleCollider>("newCapsuleCollider");
  world3D.raw_method<lWorld3DAddCollider>("addCollider");
  world3D.raw_method<lWorld3DRaycastClosest>("raycastClosest");
  world3D.method<lWorld3DRemoveCollider>("removeCollider");
  world3D.method<lWorld3DDestroyCollider>("destroyCollider");
  world3D.method<lWorld3DSetGravity>("setGravity");
  world3D.method<lWorld3DGetGravity>("getGravity");
  world3D.method<lWorld3DBodyCount>("bodyCount");
  world3D.method<lWorld3DOptimizeBroadPhase>("optimizeBroadPhase");
  world3D.method<lWorld3DUpdate>("update");

  world2D.static_function<lWorld2DNew>("new");

  world3D.static_function<lWorld3DNew>("new");
}
} // namespace sinen
