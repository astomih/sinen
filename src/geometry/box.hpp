#ifndef SINEN_BOX_HPP
#define SINEN_BOX_HPP
#include "mesh.hpp"
#include <core/data/ptr.hpp>
#include <math/vector.hpp>
namespace sinen {
struct AABB {
  Vec3 min;
  Vec3 max;
  AABB() : min(Vec3(0.0f)), max(Vec3(0.0f)) {}
  ~AABB() = default;
  /**
   * @brief Construct a new aabb object
   *
   * @param min  min point
   * @param max  max point
   */
  AABB(const Vec3 &min, const Vec3 max) : min(min), max(max) {}
  void updateWorld(const Vec3 &p, const Vec3 &scale, const AABB &local);

  Ptr<Mesh> createMesh();
};
/**
 * @brief OBBox(Oriented Bounding Box) class
 *
 */
struct OBB {
  OBB() = default;
  ~OBB() = default;
  // Position
  Vec3 p;
  // Direct vectors
  Vec3 v[3];
  // Axis lengths
  float a[3];
};
} // namespace sinen
#endif