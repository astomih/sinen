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

#include "world3d.hpp"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
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

  tempAllocator = makeUnique<TempAllocator>();
  jobSystem = makeUnique<JPH::JobSystemThreadPool>(
      JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
      std::thread::hardware_concurrency() - 1);

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
  tempAllocator = nullptr;
}

static UInt32 nextColliderID = 1;
UInt32 getNextId() { return nextColliderID++; }

class World3DImpl : public World3D {
  Hashmap<UInt32, JPH::BodyID> bodyMap = {};
  UniquePtr<JPH::PhysicsSystem> physicsSystem;

  bool tryGetBodyID(const Collider &collider, JPH::BodyID &out) {
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return false;
    }
    out = it->second;
    return true;
  }

public:
  World3DImpl() {
    const UInt32 cMaxBodies = 1024;
    const UInt32 cNumBodyMutexes = 0;
    const UInt32 cMaxBodyPairs = 1024;
    const UInt32 cMaxContactConstraints = 1024;
    static BPLayerInterfaceImpl broadPhaseLayerInterface;
    static ObjectVsBroadPhaseLayerFilterImpl objectVSBroadPhaseLayerFilter;
    static ObjectLayerPairFilterImpl objectVSLayerFilter;

    physicsSystem = makeUnique<JPH::PhysicsSystem>();
    physicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
                        cMaxContactConstraints, broadPhaseLayerInterface,
                        objectVSBroadPhaseLayerFilter, objectVSLayerFilter);

    static MyBodyActivationListener bodyActivationListener;
    physicsSystem->SetBodyActivationListener(&bodyActivationListener);

    static MyContactListener contactListener;
    physicsSystem->SetContactListener(&contactListener);
  }
  ~World3DImpl() override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    for (auto &id : bodyMap) {
      if (bodyInterface.IsAdded(id.second)) {
        bodyInterface.RemoveBody(id.second);
      }
      bodyInterface.DestroyBody(id.second);
    }
    bodyMap.clear();
  }

  bool isValid(const Collider &collider) override {
    return bodyMap.find(collider.id) != bodyMap.end();
  }
  bool isAdded(const Collider &collider) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return false;
    }
    return physicsSystem->GetBodyInterface().IsAdded(bodyID);
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

  Vec3 getRotation(const Collider &collider) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return Vec3(0.0f);
    }
    JPH::Quat r = physicsSystem->GetBodyInterface().GetRotation(bodyID);
    const Vec3 eRad =
        Quat::toEuler(Quat(r.GetX(), r.GetY(), r.GetZ(), r.GetW()));
    return {Math::toDegrees(eRad.x), Math::toDegrees(eRad.y),
            Math::toDegrees(eRad.z)};
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

  Vec3 getAngularVelocity(const Collider &collider) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return Vec3(0.0f);
    }
    auto w = physicsSystem->GetBodyInterface().GetAngularVelocity(bodyID);
    return Vec3(w.GetX(), w.GetY(), w.GetZ());
  }

  void setPosition(const Collider &collider, const Vec3 &position,
                   bool activate) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().SetPosition(
        bodyID, {position.x, position.y, position.z},
        activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
  }

  void setRotation(const Collider &collider, const Vec3 &rotation,
                   bool activate) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    const auto q = Quat::fromEuler(rotation);
    physicsSystem->GetBodyInterface().SetRotation(
        bodyID, {q.x, q.y, q.z, q.w},
        activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
  }

  void setPositionAndRotation(const Collider &collider, const Vec3 &position,
                              const Vec3 &rotation, bool activate) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    const auto q = Quat::fromEuler(rotation);
    physicsSystem->GetBodyInterface().SetPositionAndRotation(
        bodyID, {position.x, position.y, position.z}, {q.x, q.y, q.z, q.w},
        activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
  }

  void setLinearVelocity(const Collider &collider,
                         const Vec3 &velocity) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return;
    }
    auto bodyID = it->second;
    bodyInterface.SetLinearVelocity(bodyID,
                                    {velocity.x, velocity.y, velocity.z});
  }

  void setAngularVelocity(const Collider &collider,
                          const Vec3 &velocity) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return;
    }
    bodyInterface.SetAngularVelocity(it->second,
                                     {velocity.x, velocity.y, velocity.z});
  }

  void addForce(const Collider &collider, const Vec3 &force,
                bool activate) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().AddForce(
        bodyID, {force.x, force.y, force.z},
        activate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
  }

  void addImpulse(const Collider &collider, const Vec3 &impulse) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().AddImpulse(
        bodyID, {impulse.x, impulse.y, impulse.z});
  }

  void setFriction(const Collider &collider, float friction) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().SetFriction(bodyID, friction);
  }

  void setRestitution(const Collider &collider, float restitution) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().SetRestitution(bodyID, restitution);
  }

  void activate(const Collider &collider) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().ActivateBody(bodyID);
  }

  void deactivate(const Collider &collider) override {
    JPH::BodyID bodyID;
    if (!tryGetBodyID(collider, bodyID)) {
      return;
    }
    physicsSystem->GetBodyInterface().DeactivateBody(bodyID);
  }
  void setGravity(const Vec3 &gravity) override {
    if (!physicsSystem) {
      return;
    }
    physicsSystem->SetGravity({gravity.x, gravity.y, gravity.z});
  }
  Vec3 getGravity() override {
    if (!physicsSystem) {
      return Vec3(0.0f);
    }
    auto g = physicsSystem->GetGravity();
    return {g.GetX(), g.GetY(), g.GetZ()};
  }
  UInt32 bodyCount() override {
    return physicsSystem ? static_cast<UInt32>(physicsSystem->GetNumBodies())
                         : 0;
  }

  Collider newBoxCollider(const Transform &transform, bool isStatic) override {

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

  Collider newSphereCollider(const Vec3 &position, float radius,
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
  Collider newCylinderCollider(const Vec3 &position, const Vec3 &rotation,
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

  Collider newCapsuleCollider(const Vec3 &position, const Vec3 &rotation,
                              float halfHeight, float radius,
                              bool isStatic) override {
    auto &bodyInterface = physicsSystem->GetBodyInterface();
    JPH::CapsuleShapeSettings capsuleShapeSetting(halfHeight, radius);
    capsuleShapeSetting.SetEmbedded();
    JPH::EMotionType motionType =
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer = isStatic ? Layers::nonMoving : Layers::moving;
    JPH::ShapeSettings::ShapeResult capsuleShapeResult =
        capsuleShapeSetting.Create();
    JPH::ShapeRefC capsuleShape = capsuleShapeResult.Get();
    const auto quaternion = Quat::fromEuler(rotation);
    JPH::BodyCreationSettings capsuleSettings(
        capsuleShape, JPH::RVec3(position.x, position.y, position.z),
        {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
        layer);
    JPH::Body *capsule = bodyInterface.CreateBody(capsuleSettings);
    Collider collider{*this, getNextId()};
    bodyMap[collider.id] = capsule->GetID();
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

  void removeCollider(const Collider &collider) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return;
    }
    if (!bodyInterface.IsAdded(it->second)) {
      return;
    }
    bodyInterface.RemoveBody(it->second);
  }

  void destroyCollider(const Collider &collider) override {
    JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
    auto it = bodyMap.find(collider.id);
    if (it == bodyMap.end()) {
      return;
    }
    if (bodyInterface.IsAdded(it->second)) {
      bodyInterface.RemoveBody(it->second);
    }
    bodyInterface.DestroyBody(it->second);
    bodyMap.erase(it);
  }
  void optimizeBroadPhase() override { physicsSystem->OptimizeBroadPhase(); }

  void update(float time, int collisionSteps) override {
    physicsSystem->Update(time, collisionSteps, tempAllocator.get(),
                          jobSystem.get());
  }
};

Ptr<World3D> World3D::create() { return makePtr<World3DImpl>(); }

Collider::Collider(World3D &world, UInt32 id) : world(world), id(id) {}
bool Collider::isValid() const { return world.isValid(*this); }
bool Collider::isAdded() const { return world.isAdded(*this); }
Vec3 Collider::getPosition() const { return world.getPosition(*this); }
Vec3 Collider::getRotation() const { return world.getRotation(*this); }
Vec3 Collider::getVelocity() const { return world.getVelocity(*this); }
Vec3 Collider::getAngularVelocity() const {
  return world.getAngularVelocity(*this);
}
void Collider::setPosition(const Vec3 &position, bool activate) const {
  world.setPosition(*this, position, activate);
}
void Collider::setRotation(const Vec3 &rotation, bool activate) const {
  world.setRotation(*this, rotation, activate);
}
void Collider::setPositionAndRotation(const Vec3 &position,
                                      const Vec3 &rotation,
                                      bool activate) const {
  world.setPositionAndRotation(*this, position, rotation, activate);
}
void Collider::setLinearVelocity(const Vec3 &velocity) const {
  world.setLinearVelocity(*this, velocity);
}
void Collider::setAngularVelocity(const Vec3 &velocity) const {
  world.setAngularVelocity(*this, velocity);
}
void Collider::addForce(const Vec3 &force, bool activate) const {
  world.addForce(*this, force, activate);
}
void Collider::addImpulse(const Vec3 &impulse) const {
  world.addImpulse(*this, impulse);
}
void Collider::setFriction(float friction) const {
  world.setFriction(*this, friction);
}
void Collider::setRestitution(float restitution) const {
  world.setRestitution(*this, restitution);
}
void Collider::activate() const { world.activate(*this); }
void Collider::deactivate() const { world.deactivate(*this); }
void Collider::remove() const { world.removeCollider(*this); }
void Collider::destroy() const { world.destroyCollider(*this); }

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
  luaPushcfunction2(L, udPtrGc<World3D>);
  lua_setfield(L, -2, "__gc");
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
