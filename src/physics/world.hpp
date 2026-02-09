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
  virtual Vec3 getPosition(const Collider &collider) = 0;
  virtual Vec3 getVelocity(const Collider &collider) = 0;
  virtual void setLinearVelocity(const Collider &collider,
                                 const Vec3 &velocity) = 0;
  virtual Collider createBoxCollider(const Transform &transform,
                                     bool isStatic) = 0;
  virtual Collider createSphereCollider(const Vec3 &position, float radius,
                                        bool isStatic) = 0;
  virtual Collider createCylinderCollider(const Vec3 &position,
                                          const Vec3 &rotation,
                                          float halfHeight, float radius,
                                          bool isStatic) = 0;

  virtual void addCollider(const Collider &collider, bool active) = 0;
};
} // namespace sinen
#endif // SINEN_WORLD_HPP