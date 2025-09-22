#ifndef SINEN_PHYSICS_SYSTEM_HPP
#define SINEN_PHYSICS_SYSTEM_HPP
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

#include <cstdarg>
#include <iostream>
#include <memory>
#include <thread>

#include <logic/physics/physics.hpp>
#include <math/transform/transform.hpp>

namespace sinen {
class PhysicsSystem {
public:
  static bool Initialize();
  static void Shutdown();

  static void PostSetup();
  static void Update();

  static glm::vec3 GetPosition(const Collider &collider);
  static glm::vec3 GetVelocity(const Collider &collider);
  static void SetLinearVelocity(const Collider &collider,
                                const glm::vec3 &velocity);

  static Collider CreateBoxCollider(const Transform &transform, bool isStatic);
  static Collider CreateSphereCollider(const glm::vec3 &position, float radius,
                                       bool isStatic);
  static Collider CreateCylinderCollider(const glm::vec3 &position,
                                         const glm::vec3 &rotation,
                                         float halfHeight, float radius,
                                         bool isStatic);

  static void AddCollider(const Collider &collider, bool active);

private:
  struct RawData {

    RawData(size_t allocatorSize)
        : tempAllocator(allocatorSize),
          jobSystem(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                    std::thread::hardware_concurrency() - 1) {}

    JPH::PhysicsSystem physicsSystem;
    JPH::TempAllocatorImpl tempAllocator;
    JPH::JobSystemThreadPool jobSystem;
  };

  static inline std::unique_ptr<RawData> raw = nullptr;
};
} // namespace sinen

#endif // SINEN_PHYSICS_SYSTEM_HPP