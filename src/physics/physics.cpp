#include "physics.hpp"

// STL includes
#include <cstdarg>
#include <iostream>
#include <memory>
#include <thread>

#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <core/time/time.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
#include <script/luaapi.hpp>

#include "world.hpp"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

namespace sinen {
class TempAllocator : public JPH::TempAllocator {
public:
  TempAllocator() = default;
  /// Destructor
  ~TempAllocator() {}

  /// Allocates inSize bytes of memory, returned memory address must be
  /// JPH_RVECTOR_ALIGNMENT byte aligned
  void *Allocate(JPH::uint inSize) override {
    return GlobalAllocator::get()->allocate(inSize);
  }

  /// Frees inSize bytes of memory located at inAddress
  void Free(void *inAddress, JPH::uint inSize) override {
    if (inSize == 0)
      return;
    GlobalAllocator::get()->deallocate(inAddress, inSize);
  }
};
static UniquePtr<JPH::PhysicsSystem> physicsSystem;
static UniquePtr<JPH::TempAllocator> tempAllocator;
static UniquePtr<JPH::JobSystemThreadPool> jobSystem;
static void traceImplement(const char *inFMT, ...) {
  va_list list;
  va_start(list, inFMT);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), inFMT, list);
  va_end(list);
}

#ifdef JPH_ENABLE_ASSERTS

static bool assertFailedImpl(const char *inExpression, const char *inMessage,
                             const char *inFile, UInt32 inLine) {
  std::cout << inFile << ":" << inLine << ": (" << inExpression << ") "
            << (inMessage != nullptr ? inMessage : "") << std::endl;

  return true;
};

#endif // JPH_ENABLE_ASSERTS

namespace Layers {
static constexpr JPH::ObjectLayer nonMoving = 0;
static constexpr JPH::ObjectLayer moving = 1;
static constexpr JPH::ObjectLayer numLayers = 2;
}; // namespace Layers

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inObject1,
                             JPH::ObjectLayer inObject2) const override {
    switch (inObject1) {
    case Layers::nonMoving:
      return inObject2 == Layers::moving;
    case Layers::moving:
      return true;
    default:
      JPH_ASSERT(false);
      return false;
    }
  }
};

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer nonMoving(0);
static constexpr JPH::BroadPhaseLayer moving(1);
static constexpr UInt32 numLayers(2);
}; // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
  BPLayerInterfaceImpl() {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::nonMoving] = BroadPhaseLayers::nonMoving;
    mObjectToBroadPhase[Layers::moving] = BroadPhaseLayers::moving;
  }

  virtual UInt32 GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::numLayers;
  }

  virtual JPH::BroadPhaseLayer
  GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
    JPH_ASSERT(inLayer < Layers::numLayers);
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *
  GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::nonMoving:
      return "NON_MOVING";
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::moving:
      return "MOVING";
    default:
      JPH_ASSERT(false);
      return "INVALID";
    }
  }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
  JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::numLayers];
};

class ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
                             JPH::BroadPhaseLayer inLayer2) const override {
    switch (inLayer1) {
    case Layers::nonMoving:
      return inLayer2 == BroadPhaseLayers::moving;
    case Layers::moving:
      return true;
    default:
      JPH_ASSERT(false);
      return false;
    }
  }
};

class MyContactListener : public JPH::ContactListener {
public:
  virtual JPH::ValidateResult
  OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2,
                    JPH::RVec3Arg inBaseOffset,
                    const JPH::CollideShapeResult &inCollisionResult) override {
    std::cout << "Contact validate callback" << std::endl;

    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  virtual void OnContactAdded(const JPH::Body &inBody1,
                              const JPH::Body &inBody2,
                              const JPH::ContactManifold &inManifold,
                              JPH::ContactSettings &ioSettings) override {
    std::cout << "A contact was added" << std::endl;
  }

  virtual void OnContactPersisted(const JPH::Body &inBody1,
                                  const JPH::Body &inBody2,
                                  const JPH::ContactManifold &inManifold,
                                  JPH::ContactSettings &ioSettings) override {
    std::cout << "A contact was persisted" << std::endl;
  }

  virtual void
  OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override {
    std::cout << "A contact was removed" << std::endl;
  }
};

class MyBodyActivationListener : public JPH::BodyActivationListener {
public:
  virtual void OnBodyActivated(const JPH::BodyID &inBodyID,
                               UInt64 inBodyUserData) override {
    std::cout << "A body got activated" << std::endl;
  }

  virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID,
                                 UInt64 inBodyUserData) override {
    std::cout << "A body went to sleep" << std::endl;
  }
};

bool Physics::initialize() {
  JPH::RegisterDefaultAllocator();

  JPH::Trace = traceImplement;
  JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assertFailedImpl;)

  JPH::Factory::sInstance = new JPH::Factory();

  JPH::RegisterTypes();

  physicsSystem = makeUnique<JPH::PhysicsSystem>();
  tempAllocator = makeUnique<TempAllocator>();
  jobSystem = makeUnique<JPH::JobSystemThreadPool>(
      JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
      std::thread::hardware_concurrency() - 1);

  const UInt32 cMaxBodies = 1024;
  const UInt32 cNumBodyMutexes = 0;
  const UInt32 cMaxBodyPairs = 1024;
  const UInt32 cMaxContactConstraints = 1024;
  static BPLayerInterfaceImpl broadPhaseLayerInterface;
  static ObjectVsBroadPhaseLayerFilterImpl objectVSBroadPhaseLayerFilter;
  static ObjectLayerPairFilterImpl objectVSLayerFilter;

  physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
                      cMaxContactConstraints, broadPhaseLayerInterface,
                      objectVSBroadPhaseLayerFilter, objectVSLayerFilter);

  static MyBodyActivationListener bodyActivationListener;
  physicsSystem->SetBodyActivationListener(&bodyActivationListener);

  static MyContactListener contactListener;
  physicsSystem->SetContactListener(&contactListener);

  return true;
}
void Physics::shutdown() {

  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;

  JPH::UnregisterTypes();

  auto barrier = jobSystem->CreateBarrier();
  jobSystem->WaitForJobs(barrier);
  jobSystem->DestroyBarrier(barrier);

  jobSystem = nullptr;
  physicsSystem = nullptr;
  tempAllocator = nullptr;
}

void Physics::postSetup() {
  if (physicsSystem->GetNumBodies() == 0) {
    return;
  }
  physicsSystem->OptimizeBroadPhase();
}

void Physics::update() {
  const int cCollisionSteps = 1;
  physicsSystem->Update(Time::deltaTime(), cCollisionSteps, tempAllocator.get(),
                        jobSystem.get());
}

static UInt32 nextColliderID = 1;
UInt32 getNextId() { return nextColliderID++; }

class WorldImpl : public World {
  Hashmap<UInt32, JPH::BodyID> bodyMap = {};

public:
  WorldImpl() {}
  ~WorldImpl() override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    for (auto &id : bodyMap) {
      bodyInterface.RemoveBody(id.second);
      bodyInterface.DestroyBody(id.second);
    }
  }
  Vec3 getPosition(const Collider &collider) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return Vec3(0.0f);
    }
    JPH::BodyID bodyId = it->second;
    JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(bodyId);
    return {position.GetX(), position.GetY(), position.GetZ()};
  }

  Vec3 getVelocity(const Collider &collider) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return Vec3(0.0f);
    }
    JPH::BodyID bodyId = it->second;
    auto velocity = bodyInterface.GetLinearVelocity(bodyId);
    return Vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
  }

  void setLinearVelocity(const Collider &collider,
                         const Vec3 &velocity) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto bodyID = bodyMap[collider.id];
    bodyInterface.SetLinearVelocity(bodyID,
                                    {velocity.x, velocity.y, velocity.z});
  }

  Collider createBoxCollider(const Transform &transform,
                             bool isStatic) override {

    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();

    auto &position = transform.position;
    auto &rotation = transform.rotation;
    auto &scale = transform.scale;
    JPH::BoxShapeSettings boxShapeSetting({scale.x, scale.y, scale.z});
    boxShapeSetting.SetEmbedded();

    // Create the shape
    JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSetting.Create();
    JPH::ShapeRefC boxShape = boxShapeResult.Get();
    const auto quaternion = Quat::fromEuler(rotation);

    JPH::EMotionType motionType =
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;

    JPH::ObjectLayer layer = isStatic ? Layers::nonMoving : Layers::moving;

    JPH::BodyCreationSettings boxSettings(
        boxShape, JPH::RVec3(position.x, position.y, position.z),
        {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
        layer);

    JPH::Body *floor = bodyInterface.CreateBody(boxSettings);
    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = floor->GetID();
    return collider;
  }

  Collider createSphereCollider(const Vec3 &position, float radius,
                                bool isStatic) override {
    auto &bodyInterface = physicsSystem->GetBodyInterface();
    auto motionType =
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer = isStatic ? Layers::nonMoving : Layers::moving;
    JPH::BodyCreationSettings sphereSettings(
        new JPH::SphereShape(radius),
        JPH::RVec3(position.x, position.y, position.z), JPH::Quat::sIdentity(),
        motionType, layer);
    auto *body = bodyInterface.CreateBody(sphereSettings);

    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = body->GetID();
    return collider;
  }
  Collider createCylinderCollider(const Vec3 &position, const Vec3 &rotation,
                                  float halfHeight, float radius,
                                  bool isStatic) override {
    auto &bodyInterface = physicsSystem->GetBodyInterface();
    JPH::CylinderShapeSettings cylinderShapeSetting(halfHeight, radius);
    cylinderShapeSetting.SetEmbedded();
    JPH::EMotionType motionType =
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer = isStatic ? Layers::nonMoving : Layers::moving;
    JPH::ShapeSettings::ShapeResult cylinderShapeResult =
        cylinderShapeSetting.Create();
    JPH::ShapeRefC cylinderShape = cylinderShapeResult.Get();
    const auto quaternion = Quat::fromEuler(rotation);
    JPH::BodyCreationSettings cylinderSettings(
        cylinderShape, JPH::RVec3(position.x, position.y, position.z),
        {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
        layer);
    JPH::Body *cylinder = bodyInterface.CreateBody(cylinderSettings);
    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = cylinder->GetID();
    return collider;
  }
  void addCollider(const Collider &collider, bool active) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return;
    }
    JPH::BodyID bodyID = it->second;
    bodyInterface.AddBody(bodyID, active ? JPH::EActivation::Activate
                                         : JPH::EActivation::DontActivate);
  }
};

Ptr<World> World::create() { return makePtr<WorldImpl>(); }

Collider::Collider(World &world, UInt32 id) : world(world), id(id) {}
Vec3 Collider::getPosition() const { return world.getPosition(*this); }
Vec3 Collider::getVelocity() const { return world.getVelocity(*this); }
void Collider::setLinearVelocity(const Vec3 &velocity) const {
  world.setLinearVelocity(*this, velocity);
}

static int lWorldNew(lua_State *L) {
  int n = lua_gettop(L);
  udPushPtr<World>(L, World::create());
  return 1;
}

static int lPhysicsCreateBoxCollider(lua_State *L) {
  auto w = udPtr<World>(L, 1);
  auto &t = udValue<Transform>(L, 2);
  bool isStatic = lua_toboolean(L, 3) != 0;
  auto collider = w->createBoxCollider(t, isStatic);
  udNewOwned<Collider>(L, collider);
  return 1;
}
static int lPhysicsCreateSphereCollider(lua_State *L) {
  auto w = udPtr<World>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  float radius = static_cast<float>(luaL_checknumber(L, 3));
  bool isStatic = lua_toboolean(L, 4) != 0;
  udNewOwned<Collider>(L, w->createSphereCollider(pos, radius, isStatic));
  return 1;
}
static int lPhysicsCreateCylinderCollider(lua_State *L) {
  auto w = udPtr<World>(L, 1);
  auto &pos = udValue<Vec3>(L, 2);
  auto &rot = udValue<Vec3>(L, 3);
  float halfHeight = static_cast<float>(luaL_checknumber(L, 4));
  float radius = static_cast<float>(luaL_checknumber(L, 5));
  bool isStatic = lua_toboolean(L, 6) != 0;
  udNewOwned<Collider>(
      L, w->createCylinderCollider(pos, rot, halfHeight, radius, isStatic));
  return 1;
}
static int lPhysicsAddCollider(lua_State *L) {
  auto w = udPtr<World>(L, 1);
  auto &c = udValue<Collider>(L, 2);
  bool active = lua_toboolean(L, 3) != 0;
  w->addCollider(c, active);
  return 0;
}
void registerPhysics(lua_State *L) {
  luaL_newmetatable(L, World::metaTableName());
  luaPushcfunction2(L, udPtrGc<World>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lPhysicsCreateBoxCollider);
  lua_setfield(L, -2, "createBoxCollider");
  luaPushcfunction2(L, lPhysicsCreateSphereCollider);
  lua_setfield(L, -2, "createSphereCollider");
  luaPushcfunction2(L, lPhysicsCreateCylinderCollider);
  lua_setfield(L, -2, "createCylinderCollider");
  luaPushcfunction2(L, lPhysicsAddCollider);
  lua_setfield(L, -2, "addCollider");
  lua_pop(L, 1);

  pushSnNamed(L, "World");
  luaPushcfunction2(L, lWorldNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
