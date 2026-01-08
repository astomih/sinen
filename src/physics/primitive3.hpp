#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include <math/vector.hpp>

namespace sinen {
/**
 * @brief Line3D class
 *
 */
struct Line3 {
  Line3() = default;
  Line3(const Vec3 &p, const Vec3 &v) : p(p), v(v) {}
  Vec3 getPoint(float t) const { return v * t; }

  Vec3 p;
  Vec3 v;
};
/**
 * @brief Segment3D class
 *
 */
struct Segment3 : public Line3 {
  Segment3();
  Segment3(const Line3 &l) : Line3(l) {}
  Segment3(const Vec3 &p, const Vec3 &v);
  /**
   * @brief Get end point of segment
   *
   */
  Vec3 getEndPoint() const;
};
/**
 * @brief Ray3D class
 *
 */
struct Ray3 : public Line3 {
  Ray3();
  Ray3(const Line3 &l) : Line3(l) {}
  Ray3(const Segment3 &s) : Line3(s) {}
  Ray3(const Vec3 &p, const Vec3 &v);
  /**
   * @brief Get end point of ray
   *
   */
  Vec3 getEndPoint() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle3 {
  Triangle3();
  Triangle3(const Vec3 &a, const Vec3 &b, const Vec3 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  Vec3 getNormal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float getArea() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  Vec3 getCentroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  Vec3 getBarycentric(const Vec3 &p) const;

  // vertices
  Vec3 a;
  Vec3 b;
  Vec3 c;
};
/**
 * @brief Plane3D class
 *
 */
struct Plane3 {
  Plane3() = default;
  Plane3(const Vec3 &p, const Vec3 &n) : p(p), n(n) {}

  // point on plane
  Vec3 p;
  // normal of plane
  Vec3 n;
};
/**
 * @brief Sphere class
 *
 */
struct Sphere {
  Sphere() : r(0.f) {};
  ~Sphere() = default;
  Sphere(const Vec3 &p, float r) : p(p), r(r) {}

  // position
  Vec3 p;
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
  Capsule(const Vec3 &p1, const Vec3 &p2, float r) : s(p1, p2), r(r) {}
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
} // namespace sinen
#endif // !SINEN_PRIMITIVE_HPP
