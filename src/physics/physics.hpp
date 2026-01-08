#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP

#include "collision.hpp"
#include "primitive2.hpp"
#include "primitive3.hpp"
#include <math/transform/transform.hpp>

namespace sinen {
class Collider {
public:
  Vec3 getPosition() const;
  Vec3 getVelocity() const;
  void setLinearVelocity(const Vec3 &velocity) const;
  uint32_t id;
};
/**
 * @brief Physics class
 *
 */
class Physics {
public:
  static Collider createBoxCollider(const Transform &transform, bool isStatic);
  static Collider createSphereCollider(const Vec3 &position, float radius,
                                       bool isStatic);
  static Collider createCylinderCollider(const Vec3 &position,
                                         const Vec3 &rotation, float halfHeight,
                                         float radius, bool isStatic);

  static void addCollider(const Collider &collider, bool active = true);
};
} // namespace sinen

#endif // !SINEN_PHYSICS_HPP