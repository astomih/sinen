#ifndef SINEN_WORLD_HPP
#define SINEN_WORLD_HPP
#include <core/data/ptr.hpp>
#include <math/transform/transform.hpp>

#include "collider.hpp"

namespace sinen {
class World {
public:
  static Ptr<World> create();
  static constexpr const char *metaTableName() { return "sn.World"; }
  virtual ~World() = default;
  virtual bool isValid(const Collider &collider) = 0;
  virtual bool isAdded(const Collider &collider) = 0;
  virtual Vec3 getPosition(const Collider &collider) = 0;
  virtual Vec3 getRotation(const Collider &collider) = 0;
  virtual Vec3 getVelocity(const Collider &collider) = 0;
  virtual Vec3 getAngularVelocity(const Collider &collider) = 0;
  virtual void setPosition(const Collider &collider, const Vec3 &position,
                           bool activate) = 0;
  virtual void setRotation(const Collider &collider, const Vec3 &rotation,
                           bool activate) = 0;
  virtual void setPositionAndRotation(const Collider &collider,
                                      const Vec3 &position,
                                      const Vec3 &rotation,
                                      bool activate) = 0;
  virtual void setLinearVelocity(const Collider &collider,
                                 const Vec3 &velocity) = 0;
  virtual void setAngularVelocity(const Collider &collider,
                                  const Vec3 &velocity) = 0;
  virtual void addForce(const Collider &collider, const Vec3 &force,
                        bool activate) = 0;
  virtual void addImpulse(const Collider &collider, const Vec3 &impulse) = 0;
  virtual void setFriction(const Collider &collider, float friction) = 0;
  virtual void setRestitution(const Collider &collider, float restitution) = 0;
  virtual void activate(const Collider &collider) = 0;
  virtual void deactivate(const Collider &collider) = 0;
  virtual Collider createBoxCollider(const Transform &transform,
                                     bool isStatic) = 0;
  virtual Collider createSphereCollider(const Vec3 &position, float radius,
                                        bool isStatic) = 0;
  virtual Collider createCylinderCollider(const Vec3 &position,
                                          const Vec3 &rotation,
                                          float halfHeight, float radius,
                                          bool isStatic) = 0;
  virtual Collider createCapsuleCollider(const Vec3 &position,
                                         const Vec3 &rotation, float halfHeight,
                                         float radius, bool isStatic) = 0;

  virtual void addCollider(const Collider &collider, bool active) = 0;
  virtual void removeCollider(const Collider &collider) = 0;
  virtual void destroyCollider(const Collider &collider) = 0;
};
} // namespace sinen
#endif // SINEN_WORLD_HPP
