#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP
#include "collider.hpp"

#include <math/transform/transform.hpp>

namespace sinen {
class Physics {
public:
  static bool initialize();
  static void shutdown();

  static void postSetup();
  static void update();

  static void setGravity(const Vec3 &gravity);
  static Vec3 getGravity();
  static UInt32 bodyCount();
};
} // namespace sinen
#endif // SINEN_PHYSICS_HPP
