#ifndef SINEN_COLLIDER_HPP
#define SINEN_COLLIDER_HPP
#include <core/data/ptr.hpp>
#include <math/vec3.hpp>
namespace sinen {
class World;
class Collider {
public:
  Collider(World &world, UInt32 id);
  static constexpr const char *metaTableName() { return "sn.Collider"; }
  Vec3 getPosition() const;
  Vec3 getVelocity() const;
  void setLinearVelocity(const Vec3 &velocity) const;
  UInt32 id;

private:
  World &world;
};
} // namespace sinen

#endif
