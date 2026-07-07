#ifndef SINEN_WORLD3D_HPP
#define SINEN_WORLD3D_HPP
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <math/transform/transform.hpp>

#include "collider.hpp"

namespace sinen {
struct RaycastHit3D {
  UInt32 colliderId = 0;
  Vec3 point = Vec3(0.0f);
  Vec3 normal = Vec3(0.0f);
  float fraction = 0.0f;
  float distance = 0.0f;
  UInt64 userMaterialId = 0;
  int triangleIndex = -1;
  int childIndex = -1;
};

class World3D : public PhysicsWorld {
public:
  static Ptr<World3D> create();
  static constexpr const char *metaTableName() { return "sn.World3D"; }
  virtual ~World3D() = default;
  virtual void setGravity(const Vec3 &gravity) = 0;
  virtual Vec3 getGravity() = 0;
  virtual UInt32 bodyCount() = 0;
  virtual Collider newBoxCollider(const Transform &transform,
                                  bool isStatic) = 0;
  virtual Collider newSphereCollider(const Vec3 &position, float radius,
                                     bool isStatic) = 0;
  virtual Collider newCylinderCollider(const Vec3 &position,
                                       const Vec3 &rotation, float halfHeight,
                                       float radius, bool isStatic) = 0;
  virtual Collider newCapsuleCollider(const Vec3 &position,
                                      const Vec3 &rotation, float halfHeight,
                                      float radius, bool isStatic) = 0;

  virtual void addCollider(const Collider &collider, bool active) = 0;
  virtual bool raycastClosest(const Vec3 &origin, const Vec3 &direction,
                              float maxDistance, RaycastHit3D &hit) = 0;
  virtual void optimizeBroadPhase() = 0;
  virtual void update(float time, int collisionSteps) = 0;
};
} // namespace sinen
#endif // SINEN_WORLD3D_HPP
