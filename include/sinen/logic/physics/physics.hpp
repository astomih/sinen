#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP

#include <glm/vec3.hpp>

namespace sinen {
class Collider {
public:
  glm::vec3 GetPosition() const;
  glm::vec3 GetVelocity() const;
  uint32_t id;
};
/**
 * @brief Physics class
 *
 */
class Physics {
public:
  static Collider CreateBoxCollider();
  static Collider CreateSphereCollider();
};
} // namespace sinen

#endif // !SINEN_PHYSICS_HPP