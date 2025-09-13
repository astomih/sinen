#include "physics_system.hpp"
#include <logic/physics/physics.hpp>

namespace sinen {
glm::vec3 Collider::GetPosition() const {
  return PhysicsSystem::GetPosition(*this);
}
glm::vec3 Collider::GetVelocity() const {
  return PhysicsSystem::GetVelocity(*this);
}
Collider Physics::CreateBoxCollider() {
  return PhysicsSystem::CreateBoxCollider();
}

Collider Physics::CreateSphereCollider() {
  return PhysicsSystem::CreateSphereCollider();
}
} // namespace sinen