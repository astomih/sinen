#include "physics_system.hpp"
#include <physics/physics.hpp>

namespace sinen {
Vec3 Collider::getPosition() const { return PhysicsSystem::getPosition(*this); }
Vec3 Collider::getVelocity() const { return PhysicsSystem::getVelocity(*this); }
void Collider::setLinearVelocity(const Vec3 &velocity) const {
  PhysicsSystem::setLinearVelocity(*this, velocity);
}
Collider Physics::createBoxCollider(const Transform &transform, bool isStatic) {
  return PhysicsSystem::createBoxCollider(transform, isStatic);
}

Collider Physics::createSphereCollider(const Vec3 &position, float radius,
                                       bool isStatic) {
  return PhysicsSystem::createSphereCollider(position, radius, isStatic);
}
Collider Physics::createCylinderCollider(const Vec3 &position,
                                         const Vec3 &rotation, float halfHeight,
                                         float radius, bool isStatic) {
  return PhysicsSystem::createCylinderCollider(position, rotation, halfHeight,
                                               radius, isStatic);
}
void Physics::addCollider(const Collider &collider, bool active) {
  PhysicsSystem::addCollider(collider, active);
}
} // namespace sinen