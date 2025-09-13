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
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <cstdarg>
#include <iostream>
#include <memory>
#include <thread>

#include <logic/physics/physics.hpp>

namespace sinen {
class PhysicsSystem {
public:
  static bool Initialize();
  static void Shutdown();

  static void PostSetup();
  static void Update();

  static glm::vec3 GetPosition(const Collider &collider);
  static glm::vec3 GetVelocity(const Collider &collider);

  static Collider CreateBoxCollider();
  static Collider CreateSphereCollider();

private:
  struct RawData {

    RawData(size_t allocatorSize)
        : temp_allocator(allocatorSize),
          job_system(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                     std::thread::hardware_concurrency() - 1) {}

    JPH::PhysicsSystem physics_system;
    JPH::TempAllocatorImpl temp_allocator;
    JPH::JobSystemThreadPool job_system;
  };

  static inline std::unique_ptr<RawData> raw = nullptr;
};
} // namespace sinen

#endif // SINEN_PHYSICS_SYSTEM_HPP