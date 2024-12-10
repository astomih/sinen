#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include "../math/point3.hpp"
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Line3D class
 *
 */
struct Line3 {
  Line3() = default;
  Line3(const Vector3 &p, const Vector3 &v) : p(p), v(v) {}
  Vector3 get_point(float t) const { return v * t; }

  Vector3 p;
  Vector3 v;
};
/**
 * @brief Segment3D class
 *
 */
struct Segment3 : public Line3 {
  Segment3();
  Segment3(const Line3 &l) : Line3(l) {}
  Segment3(const Vector3 &p, const Vector3 &v);
  /**
   * @brief Get end point of segment
   *
   */
  Point3f get_end_point() const;
};
/**
 * @brief Ray3D class
 *
 */
struct Ray3 : public Line3 {
  Ray3();
  Ray3(const Line3 &l) : Line3(l) {}
  Ray3(const Segment3 &s) : Line3(s) {}
  Ray3(const Vector3 &p, const Vector3 &v);
  /**
   * @brief Get end point of ray
   *
   */
  Point3f get_end_point() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle3 {
  Triangle3();
  Triangle3(const Vector3 &a, const Vector3 &b, const Vector3 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  Vector3 get_normal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float get_area() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  Point3f get_centroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  Vector3 get_barycentric(const Vector3 &p) const;

  // vertices
  Vector3 a;
  Vector3 b;
  Vector3 c;
};
/**
 * @brief Plane3D class
 *
 */
struct Plane3 {
  Plane3() = default;
  Plane3(const Vector3 &p, const Vector3 &n) : p(p), n(n) {}

  // point on plane
  Vector3 p;
  // normal of plane
  Vector3 n;
};
/**
 * @brief Sphere class
 *
 */
struct Sphere {
  Sphere() : r(0.f){};
  ~Sphere() = default;
  Sphere(const Vector3 &p, float r) : p(p), r(r) {}

  // position
  Vector3 p;
  // radius
  float r;
};
/**
 * @brief Capsule class
 *
 */
struct Capsule {
  Capsule() : r(0.f) {}
  Capsule(const Segment3 &s, float r) : s(s), r(r) {}
  Capsule(const Vector3 &p1, const Vector3 &p2, float r) : s(p1, p2), r(r) {}
  ~Capsule() {}

  // segment
  Segment3 s;
  // radius
  float r;
};
/**
 * @brief AABB(Axis-Aligned Bounding Box) class
 *
 */
struct AABB {
  Vector3 _min;
  Vector3 _max;
  AABB() = default;
  ~AABB() = default;
  /**
   * @brief Construct a new aabb object
   *
   * @param min  min point
   * @param max  max point
   */
  AABB(const Vector3 &_min, const Vector3 &_max) : _min(_min), _max(_max) {}
  void update_world(const Vector3 &p, const Vector3 &scale, const AABB &local);
};
/**
 * @brief OBBox(Oriented Bounding Box) class
 *
 */
struct OBB {
  OBB() = default;
  ~OBB() = default;
  // Position
  Vector3 p;
  // Direct vectors
  Vector3 v[3];
  // Axis lengths
  float a[3];
};
} // namespace sinen
#endif // !SINEN_PRIMITIVE_HPP
