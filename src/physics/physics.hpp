#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP
#include "collider.hpp"
#include "collision.hpp"
#include "primitive2.hpp"
#include "primitive3.hpp"

#include <cstdarg>
#include <iostream>
#include <memory>
#include <thread>

#include <math/transform/transform.hpp>

namespace sinen {
class Physics {
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
  class RawData;
  static std::unique_ptr<RawData> raw;
};
} // namespace sinen
#endif // SINEN_PHYSICS_HPP