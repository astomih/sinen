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

#include <math/transform/transform.hpp>
#include <physics/physics.hpp>

namespace sinen {
class PhysicsSystem {
public:
  static bool initialize();
  static void shutdown();

  static void postSetup();
  static void update();

  static Vec3 getPosition(const Collider &collider);
  static Vec3 getVelocity(const Collider &collider);
  static void setLinearVelocity(const Collider &collider, const Vec3 &velocity);

  static Collider createBoxCollider(const Transform &transform, bool isStatic);
  static Collider createSphereCollider(const Vec3 &position, float radius,
                                       bool isStatic);
  static Collider createCylinderCollider(const Vec3 &position,
                                         const Vec3 &rotation, float halfHeight,
                                         float radius, bool isStatic);

  static void addCollider(const Collider &collider, bool active);

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