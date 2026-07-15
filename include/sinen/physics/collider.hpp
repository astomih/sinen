#ifndef SINEN_COLLIDER_HPP
#define SINEN_COLLIDER_HPP
#include <core/data/ptr.hpp>
#include <math/vec3.hpp>
namespace sinen {
class PhysicsWorld {
public:
  virtual ~PhysicsWorld() = default;
  virtual bool isValid(const class Collider &collider) = 0;
  virtual bool isAdded(const class Collider &collider) = 0;
  virtual Vec3 getPosition(const class Collider &collider) = 0;
  virtual Vec3 getRotation(const class Collider &collider) = 0;
  virtual Vec3 getVelocity(const class Collider &collider) = 0;
  virtual Vec3 getAngularVelocity(const class Collider &collider) = 0;
  virtual void setPosition(const class Collider &collider, const Vec3 &position,
                           bool activate) = 0;
  virtual void setRotation(const class Collider &collider, const Vec3 &rotation,
                           bool activate) = 0;
  virtual void setPositionAndRotation(const class Collider &collider,
                                      const Vec3 &position,
                                      const Vec3 &rotation, bool activate) = 0;
  virtual void setLinearVelocity(const class Collider &collider,
                                 const Vec3 &velocity) = 0;
  virtual void setAngularVelocity(const class Collider &collider,
                                  const Vec3 &velocity) = 0;
  virtual void addForce(const class Collider &collider, const Vec3 &force,
                        bool activate) = 0;
  virtual void addImpulse(const class Collider &collider,
                          const Vec3 &impulse) = 0;
  virtual void setFriction(const class Collider &collider, float friction) = 0;
  virtual void setRestitution(const class Collider &collider,
                              float restitution) = 0;
  virtual void activate(const class Collider &collider) = 0;
  virtual void deactivate(const class Collider &collider) = 0;
  virtual void removeCollider(const class Collider &collider) = 0;
  virtual void destroyCollider(const class Collider &collider) = 0;
};

class Collider {
public:
  Collider(PhysicsWorld &world, UInt32 id);
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
  PhysicsWorld &world;
};
} // namespace sinen

#endif
