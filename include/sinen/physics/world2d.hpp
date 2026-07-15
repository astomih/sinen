#ifndef SINEN_WORLD2D_HPP
#define SINEN_WORLD2D_HPP
#include <core/data/ptr.hpp>
#include <math/transform/transform.hpp>

#include "collider.hpp"

namespace sinen {
class World2D : public PhysicsWorld {
public:
  static Ptr<World2D> create();
  static constexpr const char *metaTableName() { return "sn.World2D"; }
  virtual ~World2D() = default;
  virtual void setGravity(const Vec3 &gravity) = 0;
  virtual Vec3 getGravity() = 0;
  virtual UInt32 bodyCount() = 0;
  virtual Collider newBoxCollider(const Transform &transform,
                                  bool isStatic) = 0;
  virtual Collider newCircleCollider(const Vec3 &position, float radius,
                                     bool isStatic) = 0;
  virtual Collider newCapsuleCollider(const Vec3 &position,
                                      const Vec3 &rotation, float halfHeight,
                                      float radius, bool isStatic) = 0;

  virtual void addCollider(const Collider &collider, bool active) = 0;
  virtual void optimizeBroadPhase() = 0;
  virtual void update(float time, int collisionSteps) = 0;
};
} // namespace sinen
#endif // SINEN_WORLD2D_HPP
