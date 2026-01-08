#ifndef SINEN_COLLIDER_HPP
#define SINEN_COLLIDER_HPP
#include <math/vec3.hpp>
namespace sinen {
class Collider {
public:
  Vec3 getPosition() const;
  Vec3 getVelocity() const;
  void setLinearVelocity(const Vec3 &velocity) const;
  uint32_t id;
};
} // namespace sinen

#endif