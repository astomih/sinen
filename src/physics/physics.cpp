#include "physics_system.hpp"
#include <physics/physics.hpp>

namespace sinen {
glm::vec3 Collider::GetPosition() const {
  return PhysicsSystem::GetPosition(*this);
}
glm::vec3 Collider::GetVelocity() const {
  return PhysicsSystem::GetVelocity(*this);
}
void Collider::SetLinearVelocity(const glm::vec3 &velocity) const {
  PhysicsSystem::SetLinearVelocity(*this, velocity);
}
Collider Physics::CreateBoxCollider(const Transform &transform, bool isStatic) {
  return PhysicsSystem::CreateBoxCollider(transform, isStatic);
}

Collider Physics::CreateSphereCollider(const glm::vec3 &position, float radius,
                                       bool isStatic) {
  return PhysicsSystem::CreateSphereCollider(position, radius, isStatic);
}
Collider Physics::CreateCylinderCollider(const glm::vec3 &position,
                                         const glm::vec3 &rotation,
                                         float halfHeight, float radius,
                                         bool isStatic) {
  return PhysicsSystem::CreateCylinderCollider(position, rotation, halfHeight,
                                               radius, isStatic);
}
void Physics::AddCollider(const Collider &collider, bool active) {
  PhysicsSystem::AddCollider(collider, active);
}
} // namespace sinen