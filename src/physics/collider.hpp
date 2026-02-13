#ifndef SINEN_COLLIDER_HPP
#define SINEN_COLLIDER_HPP
#include <core/data/ptr.hpp>
#include <math/vec3.hpp>
namespace sinen {
class World3D;
class Collider {
public:
  Collider(World3D &world, UInt32 id);
  static constexpr const char *metaTableName() { return "sn.Collider"; }
  bool isValid() const;
  bool isAdded() const;
  Vec3 getPosition() const;
  Vec3 getRotation() const;
  Vec3 getVelocity() const;
  Vec3 getAngularVelocity() const;
  void setPosition(const Vec3 &position, bool activate = true) const;
  void setRotation(const Vec3 &rotation, bool activate = true) const;
  void setPositionAndRotation(const Vec3 &position, const Vec3 &rotation,
                              bool activate = true) const;
  void setLinearVelocity(const Vec3 &velocity) const;
  void setAngularVelocity(const Vec3 &velocity) const;
  void addForce(const Vec3 &force, bool activate = true) const;
  void addImpulse(const Vec3 &impulse) const;
  void setFriction(float friction) const;
  void setRestitution(float restitution) const;
  void activate() const;
  void deactivate() const;
  void remove() const;
  void destroy() const;
  UInt32 id;

private:
  World3D &world;
};
} // namespace sinen

#endif
