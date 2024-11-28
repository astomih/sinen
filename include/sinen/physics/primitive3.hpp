#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include "../math/point3.hpp"
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Line3D class
 *
 */
struct line3 {
  line3() = default;
  line3(const vector3 &p, const vector3 &v) : p(p), v(v) {}
  vector3 get_point(float t) const { return v * t; }

  vector3 p;
  vector3 v;
};
/**
 * @brief Segment3D class
 *
 */
struct segment3 : public line3 {
  segment3();
  segment3(const line3 &l) : line3(l) {}
  segment3(const vector3 &p, const vector3 &v);
  /**
   * @brief Get end point of segment
   *
   */
  point3f get_end_point() const;
};
/**
 * @brief Ray3D class
 *
 */
struct ray3 : public line3 {
  ray3();
  ray3(const line3 &l) : line3(l) {}
  ray3(const segment3 &s) : line3(s) {}
  ray3(const vector3 &p, const vector3 &v);
  /**
   * @brief Get end point of ray
   *
   */
  point3f get_end_point() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct triangle3 {
  triangle3();
  triangle3(const vector3 &a, const vector3 &b, const vector3 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  vector3 get_normal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float get_area() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  point3f get_centroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  vector3 get_barycentric(const vector3 &p) const;

  // vertices
  vector3 a;
  vector3 b;
  vector3 c;
};
/**
 * @brief Plane3D class
 *
 */
struct plane3 {
  plane3() = default;
  plane3(const vector3 &p, const vector3 &n) : p(p), n(n) {}

  // point on plane
  vector3 p;
  // normal of plane
  vector3 n;
};
/**
 * @brief Sphere class
 *
 */
struct sphere {
  sphere() : r(0.f){};
  ~sphere() = default;
  sphere(const vector3 &p, float r) : p(p), r(r) {}

  // position
  vector3 p;
  // radius
  float r;
};
/**
 * @brief Capsule class
 *
 */
struct capsule {
  capsule() : r(0.f) {}
  capsule(const segment3 &s, float r) : s(s), r(r) {}
  capsule(const vector3 &p1, const vector3 &p2, float r) : s(p1, p2), r(r) {}
  ~capsule() {}

  // segment
  segment3 s;
  // radius
  float r;
};
/**
 * @brief AABB(Axis-Aligned Bounding Box) class
 *
 */
struct aabb {
  vector3 _min;
  vector3 _max;
  aabb() = default;
  ~aabb() = default;
  /**
   * @brief Construct a new aabb object
   *
   * @param min  min point
   * @param max  max point
   */
  aabb(const vector3 &_min, const vector3 &_max) : _min(_min), _max(_max) {}
  void update_world(const vector3 &p, const vector3 &scale, const aabb &local);
};
/**
 * @brief OBBox(Oriented Bounding Box) class
 *
 */
struct OBB {
  OBB() = default;
  ~OBB() = default;
  // Position
  vector3 p;
  // Direct vectors
  vector3 v[3];
  // Axis lengths
  float a[3];
};
} // namespace sinen
#endif // !SINEN_PRIMITIVE_HPP
