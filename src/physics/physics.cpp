#include "physics_system.hpp"
#include <physics/physics.hpp>

namespace sinen {
glm::vec3 Collider::getPosition() const {
  return PhysicsSystem::GetPosition(*this);
}
glm::vec3 Collider::getVelocity() const {
  return PhysicsSystem::GetVelocity(*this);
}
void Collider::setLinearVelocity(const glm::vec3 &velocity) const {
  PhysicsSystem::SetLinearVelocity(*this, velocity);
}
Collider Physics::createBoxCollider(const Transform &transform, bool isStatic) {
  return PhysicsSystem::CreateBoxCollider(transform, isStatic);
}

Collider Physics::createSphereCollider(const glm::vec3 &position, float radius,
                                       bool isStatic) {
  return PhysicsSystem::CreateSphereCollider(position, radius, isStatic);
}
Collider Physics::createCylinderCollider(const glm::vec3 &position,
                                         const glm::vec3 &rotation,
                                         float halfHeight, float radius,
                                         bool isStatic) {
  return PhysicsSystem::CreateCylinderCollider(position, rotation, halfHeight,
                                               radius, isStatic);
}
void Physics::addCollider(const Collider &collider, bool active) {
  PhysicsSystem::AddCollider(collider, active);
}
} // namespace sinen