#include "physics.hpp"

// STL includes
#include <cstdarg>
#include <iostream>
#include <memory>
#include <thread>

#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <core/time/time.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>

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
Vec3 Collider::getPosition() const { return Physics::getPosition(*this); }
Vec3 Collider::getVelocity() const { return Physics::getVelocity(*this); }
void Collider::setLinearVelocity(const Vec3 &velocity) const {
  Physics::setLinearVelocity(*this, velocity);
}
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
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}; // namespace Layers

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inObject1,
                             JPH::ObjectLayer inObject2) const override {
    switch (inObject1) {
    case Layers::NON_MOVING:
      return inObject2 == Layers::MOVING;
    case Layers::MOVING:
      return true;
    default:
      JPH_ASSERT(false);
      return false;
    }
  }
};

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr UInt32 NUM_LAYERS(2);
}; // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
  BPLayerInterfaceImpl() {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual UInt32 GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  virtual JPH::BroadPhaseLayer
  GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
    JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *
  GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
    switch ((JPH::BroadPhaseLayer::Type)inLayer) {
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
      return "NON_MOVING";
    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
      return "MOVING";
    default:
      JPH_ASSERT(false);
      return "INVALID";
    }
  }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
  JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
                             JPH::BroadPhaseLayer inLayer2) const override {
    switch (inLayer1) {
    case Layers::NON_MOVING:
      return inLayer2 == BroadPhaseLayers::MOVING;
    case Layers::MOVING:
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

inline std::unordered_map<UInt32, JPH::BodyID> bodyMap = {};
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

  static MyBodyActivationListener body_activation_listener;
  physicsSystem->SetBodyActivationListener(&body_activation_listener);

  static MyContactListener contact_listener;
  physicsSystem->SetContactListener(&contact_listener);

  return true;
}
void Physics::shutdown() {
  JPH::UnregisterTypes();

  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;

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

Vec3 Physics::getPosition(const Collider &collider) {
  JPH::BodyInterface &body_interface = physicsSystem->GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return Vec3(0.0f);
  }
  JPH::BodyID body_id = it->second;
  JPH::RVec3 position = body_interface.GetCenterOfMassPosition(body_id);
  return {position.GetX(), position.GetY(), position.GetZ()};
}

Vec3 Physics::getVelocity(const Collider &collider) {
  JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return Vec3(0.0f);
  }
  JPH::BodyID body_id = it->second;
  auto velocity = bodyInterface.GetLinearVelocity(body_id);
  return Vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
}
void Physics::setLinearVelocity(const Collider &collider,
                                const Vec3 &velocity) {
  JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
  auto bodyID = bodyMap[collider.id];
  bodyInterface.SetLinearVelocity(bodyID, {velocity.x, velocity.y, velocity.z});
}

static UInt32 nextColliderID = 1;
UInt32 GetNextID() { return nextColliderID++; }

Collider Physics::createBoxCollider(const Transform &transform, bool isStatic) {

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

  JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;

  JPH::BodyCreationSettings boxSettings(
      boxShape, JPH::RVec3(position.x, position.y, position.z),
      {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
      layer);

  JPH::Body *floor = bodyInterface.CreateBody(boxSettings);
  Collider collider{GetNextID()};
  bodyMap[collider.id] = floor->GetID();
  return collider;
}

Collider Physics::createSphereCollider(const Vec3 &position, float radius,
                                       bool isStatic) {
  auto &bodyInterface = physicsSystem->GetBodyInterface();
  auto motionType =
      isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
  JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
  JPH::BodyCreationSettings sphere_settings(
      new JPH::SphereShape(radius),
      JPH::RVec3(position.x, position.y, position.z), JPH::Quat::sIdentity(),
      motionType, layer);
  auto *body = bodyInterface.CreateBody(sphere_settings);

  Collider collider{GetNextID()};
  bodyMap[collider.id] = body->GetID();
  return collider;
}
Collider Physics::createCylinderCollider(const Vec3 &position,
                                         const Vec3 &rotation, float halfHeight,
                                         float radius, bool isStatic) {
  auto &bodyInterface = physicsSystem->GetBodyInterface();
  JPH::CylinderShapeSettings cylinderShapeSetting(halfHeight, radius);
  cylinderShapeSetting.SetEmbedded();
  JPH::EMotionType motionType =
      isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
  JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
  JPH::ShapeSettings::ShapeResult cylinderShapeResult =
      cylinderShapeSetting.Create();
  JPH::ShapeRefC cylinderShape = cylinderShapeResult.Get();
  const auto quaternion = Quat::fromEuler(rotation);
  JPH::BodyCreationSettings cylinderSettings(
      cylinderShape, JPH::RVec3(position.x, position.y, position.z),
      {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
      layer);
  JPH::Body *cylinder = bodyInterface.CreateBody(cylinderSettings);
  Collider collider{GetNextID()};
  bodyMap[collider.id] = cylinder->GetID();
  return collider;
}
void Physics::addCollider(const Collider &collider, bool active) {
  JPH::BodyInterface &bodyInterface = physicsSystem->GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return;
  }
  JPH::BodyID bodyID = it->second;
  bodyInterface.AddBody(bodyID, active ? JPH::EActivation::Activate
                                       : JPH::EActivation::DontActivate);
}
} // namespace sinen
