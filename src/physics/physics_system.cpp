#include "physics_system.hpp"
// STL includes
#include <cstdarg>
#include <iostream>
#include <thread>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/mat4x4.hpp>

#include "../core/scene/scene_system.hpp"

JPH_SUPPRESS_WARNINGS

using namespace JPH;

using namespace JPH::literals;

using namespace std;

static void TraceImpl(const char *inFMT, ...) {
  va_list list;
  va_start(list, inFMT);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), inFMT, list);
  va_end(list);

  cout << buffer << endl;
}

#ifdef JPH_ENABLE_ASSERTS

static bool AssertFailedImpl(const char *inExpression, const char *inMessage,
                             const char *inFile, uint inLine) {
  cout << inFile << ":" << inLine << ": (" << inExpression << ") "
       << (inMessage != nullptr ? inMessage : "") << endl;

  return true;
};

#endif // JPH_ENABLE_ASSERTS

namespace Layers {
static constexpr ObjectLayer NON_MOVING = 0;
static constexpr ObjectLayer MOVING = 1;
static constexpr ObjectLayer NUM_LAYERS = 2;
}; // namespace Layers

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter {
public:
  virtual bool ShouldCollide(ObjectLayer inObject1,
                             ObjectLayer inObject2) const override {
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
static constexpr BroadPhaseLayer NON_MOVING(0);
static constexpr BroadPhaseLayer MOVING(1);
static constexpr uint NUM_LAYERS(2);
}; // namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface {
public:
  BPLayerInterfaceImpl() {
    // Create a mapping table from object to broad phase layer
    mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
  }

  virtual uint GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::NUM_LAYERS;
  }

  virtual BroadPhaseLayer
  GetBroadPhaseLayer(ObjectLayer inLayer) const override {
    JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  virtual const char *
  GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override {
    switch ((BroadPhaseLayer::Type)inLayer) {
    case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
      return "NON_MOVING";
    case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
      return "MOVING";
    default:
      JPH_ASSERT(false);
      return "INVALID";
    }
  }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
  BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter {
public:
  virtual bool ShouldCollide(ObjectLayer inLayer1,
                             BroadPhaseLayer inLayer2) const override {
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

class MyContactListener : public ContactListener {
public:
  virtual ValidateResult
  OnContactValidate(const Body &inBody1, const Body &inBody2,
                    RVec3Arg inBaseOffset,
                    const CollideShapeResult &inCollisionResult) override {
    cout << "Contact validate callback" << endl;

    return ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  virtual void OnContactAdded(const Body &inBody1, const Body &inBody2,
                              const ContactManifold &inManifold,
                              ContactSettings &ioSettings) override {
    cout << "A contact was added" << endl;
  }

  virtual void OnContactPersisted(const Body &inBody1, const Body &inBody2,
                                  const ContactManifold &inManifold,
                                  ContactSettings &ioSettings) override {
    cout << "A contact was persisted" << endl;
  }

  virtual void OnContactRemoved(const SubShapeIDPair &inSubShapePair) override {
    cout << "A contact was removed" << endl;
  }
};

class MyBodyActivationListener : public BodyActivationListener {
public:
  virtual void OnBodyActivated(const BodyID &inBodyID,
                               uint64 inBodyUserData) override {
    cout << "A body got activated" << endl;
  }

  virtual void OnBodyDeactivated(const BodyID &inBodyID,
                                 uint64 inBodyUserData) override {
    cout << "A body went to sleep" << endl;
  }
};

namespace sinen {
inline std::unordered_map<uint32_t, BodyID> bodyMap = {};
bool PhysicsSystem::Initialize() {
  RegisterDefaultAllocator();

  Trace = TraceImpl;
  JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

  Factory::sInstance = new Factory();

  RegisterTypes();

  raw = std::make_unique<RawData>(10 * 1024 * 1024);
  const uint cMaxBodies = 1024;
  const uint cNumBodyMutexes = 0;
  const uint cMaxBodyPairs = 1024;
  const uint cMaxContactConstraints = 1024;
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
void PhysicsSystem::Shutdown() {
  UnregisterTypes();

  delete Factory::sInstance;
  Factory::sInstance = nullptr;
}

void PhysicsSystem::PostSetup() {
  if (raw->physicsSystem.GetNumBodies() == 0) {
    return;
  }
  raw->physicsSystem.OptimizeBroadPhase();
}

void PhysicsSystem::Update() {
  const int cCollisionSteps = 1;
  raw->physicsSystem.Update(SceneSystem::delta_time(), cCollisionSteps,
                            &raw->tempAllocator, &raw->jobSystem);
}

glm::vec3 PhysicsSystem::GetPosition(const Collider &collider) {
  BodyInterface &body_interface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return glm::vec3(0.0f);
  }
  BodyID body_id = it->second;
  RVec3 position = body_interface.GetCenterOfMassPosition(body_id);
  return {position.GetX(), position.GetY(), position.GetZ()};
}

glm::vec3 PhysicsSystem::GetVelocity(const Collider &collider) {
  BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return glm::vec3(0.0f);
  }
  BodyID body_id = it->second;
  Vec3 velocity = bodyInterface.GetLinearVelocity(body_id);
  return glm::vec3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
}
void PhysicsSystem::SetLinearVelocity(const Collider &collider,
                                      const glm::vec3 &velocity) {
  BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto bodyID = bodyMap[collider.id];
  bodyInterface.SetLinearVelocity(bodyID,
                                  Vec3(velocity.x, velocity.y, velocity.z));
}

static uint32_t nextColliderID = 1;
uint32_t GetNextID() { return nextColliderID++; }

Collider PhysicsSystem::CreateBoxCollider(const Transform &transform,
                                          bool isStatic) {

  BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();

  auto &position = transform.position;
  auto &rotation = transform.rotation;
  auto &scale = transform.scale;
  BoxShapeSettings boxShapeSetting(Vec3(scale.x, scale.y, scale.z));
  boxShapeSetting.SetEmbedded();

  // Create the shape
  ShapeSettings::ShapeResult boxShapeResult = boxShapeSetting.Create();
  ShapeRefC boxShape = boxShapeResult.Get();
  const auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  const auto rotationY =
      glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  const auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  const auto quaternion = rotationX * rotationY * rotationZ;

  EMotionType motionType =
      isStatic ? EMotionType::Static : EMotionType::Dynamic;

  ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;

  BodyCreationSettings boxSettings(
      boxShape, RVec3(position.x, position.y, position.z),
      {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
      layer);

  Body *floor = bodyInterface.CreateBody(boxSettings);
  Collider collider{GetNextID()};
  bodyMap[collider.id] = floor->GetID();
  return collider;
}

Collider PhysicsSystem::CreateSphereCollider(const glm::vec3 &position,
                                             float radius, bool isStatic) {
  BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  EMotionType motionType =
      isStatic ? EMotionType::Static : EMotionType::Dynamic;
  ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
  BodyCreationSettings sphere_settings(
      new SphereShape(radius), RVec3(position.x, position.y, position.z),
      Quat::sIdentity(), motionType, layer);
  auto *body = bodyInterface.CreateBody(sphere_settings);

  Collider collider{GetNextID()};
  bodyMap[collider.id] = body->GetID();
  return collider;
}
Collider PhysicsSystem::CreateCylinderCollider(const glm::vec3 &position,
                                               const glm::vec3 &rotation,
                                               float halfHeight, float radius,
                                               bool isStatic) {
  auto &bodyInterface = raw->physicsSystem.GetBodyInterface();
  CylinderShapeSettings cylinderShapeSetting(halfHeight, radius);
  cylinderShapeSetting.SetEmbedded();
  EMotionType motionType =
      isStatic ? EMotionType::Static : EMotionType::Dynamic;
  ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
  ShapeSettings::ShapeResult cylinderShapeResult =
      cylinderShapeSetting.Create();
  ShapeRefC cylinderShape = cylinderShapeResult.Get();
  const auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  const auto rotationY =

      glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  const auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  const auto quaternion = rotationX * rotationY * rotationZ;
  BodyCreationSettings cylinderSettings(
      cylinderShape, RVec3(position.x, position.y, position.z),
      {quaternion.x, quaternion.y, quaternion.z, quaternion.w}, motionType,
      layer);
  Body *cylinder = bodyInterface.CreateBody(cylinderSettings);
  Collider collider{GetNextID()};
  bodyMap[collider.id] = cylinder->GetID();
  return collider;
}
void PhysicsSystem::AddCollider(const Collider &collider, bool active) {
  BodyInterface &bodyInterface = raw->physicsSystem.GetBodyInterface();
  auto it = bodyMap.find(collider.id);
  if (it == bodyMap.end()) {
    return;
  }
  BodyID bodyID = it->second;
  bodyInterface.AddBody(bodyID, active ? EActivation::Activate
                                       : EActivation::DontActivate);
}
} // namespace sinen
