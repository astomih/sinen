#include "physics_system.hpp"
// STL includes
#include <cstdarg>
#include <iostream>
#include <thread>

#include <core/def/types.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>

#include "../main_system.hpp"

JPH_SUPPRESS_WARNINGS

namespace sinen {
static void traceImplement(const char *inFMT, ...) {
  va_list list;
  va_start(list, inFMT);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), inFMT, list);
  va_end(list);
}

#ifdef JPH_ENABLE_ASSERTS

static bool assertFailedImpl(const char *inExpression, const char *inMessage,
                             const char *inFile, uint32 inLine) {
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
static constexpr uint32 NUM_LAYERS(2);
}; // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
  BPLayerInterfaceImpl() {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual uint32 GetNumBroadPhaseLayers() const override {
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
                               uint64 inBodyUserData) override {
    std::cout << "A body got activated" << std::endl;
  }

  virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID,
                                 uint64 inBodyUserData) override {
    std::cout << "A body went to sleep" << std::endl;
  }
};

inline std::unordered_map<uint32_t, JPH::BodyID> bodyMap = {};
bool PhysicsSystem::initialize() {
  JPH::RegisterDefaultAllocator();

  JPH::Trace = traceImplement;
  JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assertFailedImpl;)

  JPH::Factory::sInstance = new JPH::Factory();

  JPH::RegisterTypes();

  raw = std::make_unique<RawData>(10 * 1024 * 1024);
  const uint32 cMaxBodies = 1024;
  const uint32 cNumBodyMutexes = 0;
  const uint32 cMaxBodyPairs = 1024;
  const uint32 cMaxContactConstraints = 1024;
  static BPLayerInterfaceImpl broadPhaseLayerInterface;
  static ObjectVsBroadPhaseLayerFilterImpl objectVSBroadPhaseLayerFilter;
  static ObjectLayerPairFilterImpl objectVSLayerFilter;

  raw->physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
                          cMaxContactConstraints, broadPhaseLayerInterface,
                          objectVSBroadPhaseLayerFilter, objectVSLayerFilter);

  static MyBodyActivationListener body_activation_listener;
  raw->physicsSystem.SetBodyActivationListener(&body_activation_listener);

  static MyContactListener contact_listener;
  raw->physicsSystem.SetContactListener(&contact_listener);

  return true;
}
void PhysicsSystem::shutdown() {
  JPH::UnregisterTypes();

  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::postSetup() {
  if (raw->physicsSystem.GetNumBodies() == 0) {
    return;
  }
  raw->physicsSystem.OptimizeBroadPhase();
}

void PhysicsSystem::update() {
  const int cCollisionSteps = 1;
  raw->physicsSystem.Update(MainSystem::deltaTime(), cCollisionSteps,
                            &raw->tempAllocator, &raw->jobSystem);
}

Vec3 PhysicsSystem::getPosition(const Collider &collider) {
  JPH::BodyInterface &body_interface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return glm::vec3(0.0f);
  }
  JPH::BodyID body_id = it->second;
  JPH::RVec3 position = body_interface.GetCenterOfMassPosition(body_id);
  return {position.GetX(), position.GetY(), position.GetZ()};
}

Vec3 PhysicsSystem::getVelocity(const Collider &collider) {
  JPH::BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return glm::vec3(0.0f);
  }
  JPH::BodyID body_id = it->second;
  auto velocity = bodyInterface.GetLinearVelocity(body_id);
  return glm::vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
}
void PhysicsSystem::setLinearVelocity(const Collider &collider,
                                      const glm::vec3 &velocity) {
  JPH::BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto bodyID = bodyMap[collider.id];
  bodyInterface.SetLinearVelocity(bodyID, {velocity.x, velocity.y, velocity.z});
}

static uint32_t nextColliderID = 1;
uint32_t GetNextID() { return nextColliderID++; }

Collider PhysicsSystem::createBoxCollider(const Transform &transform,
                                          bool isStatic) {

  JPH::BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();

  auto &position = transform.position;
  auto &rotation = transform.rotation;
  auto &scale = transform.scale;
  JPH::BoxShapeSettings boxShapeSetting({scale.x, scale.y, scale.z});
  boxShapeSetting.SetEmbedded();

  // Create the shape
  JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSetting.Create();
  JPH::ShapeRefC boxShape = boxShapeResult.Get();
  const auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  const auto rotationY =
      glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  const auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  const auto quaternion = rotationX * rotationY * rotationZ;

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

Collider PhysicsSystem::createSphereCollider(const glm::vec3 &position,
                                             float radius, bool isStatic) {
  auto &bodyInterface = raw->physicsSystem.GetBodyInterface();
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
Collider PhysicsSystem::createCylinderCollider(const glm::vec3 &position,
                                               const glm::vec3 &rotation,
                                               float halfHeight, float radius,
                                               bool isStatic) {
  auto &bodyInterface = raw->physicsSystem.GetBodyInterface();
  JPH::CylinderShapeSettings cylinderShapeSetting(halfHeight, radius);
  cylinderShapeSetting.SetEmbedded();
  JPH::EMotionType motionType =
      isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
  JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
  JPH::ShapeSettings::ShapeResult cylinderShapeResult =
      cylinderShapeSetting.Create();
  JPH::ShapeRefC cylinderShape = cylinderShapeResult.Get();
  const auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  const auto rotationY =

      glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  const auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  const auto quaternion = rotationX * rotationY * rotationZ;
  JPH::BodyCreationSettings cylinderSettings(
      cylinderShape, JPH::RVec3(position.x, position.y, position.z),
      {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
      layer);
  JPH::Body *cylinder = bodyInterface.CreateBody(cylinderSettings);
  Collider collider{GetNextID()};
  bodyMap[collider.id] = cylinder->GetID();
  return collider;
}
void PhysicsSystem::addCollider(const Collider &collider, bool active) {
  JPH::BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return;
  }
  JPH::BodyID bodyID = it->second;
  bodyInterface.AddBody(bodyID, active ? JPH::EActivation::Activate
                                       : JPH::EActivation::DontActivate);
}
} // namespace sinen
