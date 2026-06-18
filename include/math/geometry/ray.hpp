#ifndef SINEN_RAY_HPP
#define SINEN_RAY_HPP
#include <math/vec3.hpp>
namespace sinen {
struct Ray {
  static constexpr const char *metaTableName() { return "sn.Ray"; }
  Vec3 origin;
  Vec3 direction;
};
} // namespace sinen
#endif // !SINEN_RAY_HPP