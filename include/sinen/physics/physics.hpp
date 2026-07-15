#ifndef SINEN_PHYSICS_HPP
#define SINEN_PHYSICS_HPP
#include "collider.hpp"

#include <math/transform/transform.hpp>

namespace sinen {
class Physics {
public:
  static bool initialize();
  static void shutdown();
};
} // namespace sinen
#endif // SINEN_PHYSICS_HPP
