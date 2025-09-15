#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP

#include <math/transform/transform.hpp>

#include <glm/vec3.hpp>

namespace sinen {
class Collider {
public:
  glm::vec3 GetPosition() const;
  glm::vec3 GetVelocity() const;
  void SetLinearVelocity(const glm::vec3 &velocity) const;
  uint32_t id;
};
/**
 * @brief Physics class
 *
 */
class Physics {
public:
  static Collider CreateBoxCollider(const Transform &transform, bool isStatic);
  static Collider CreateSphereCollider(const glm::vec3 &position, float radius,
                                       bool isStatic);

  static void AddCollider(const Collider &collider, bool active = true);
};
} // namespace sinen

#endif // !SINEN_PHYSICS_HPP