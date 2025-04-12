#ifndef SINEN_PRIMITIVE3_HPP
#define SINEN_PRIMITIVE3_HPP
#include <glm/vec3.hpp>

namespace sinen {
/**
 * @brief Line3D class
 *
 */
struct Line3 {
  Line3() = default;
  Line3(const glm::vec3 &p, const glm::vec3 &v) : p(p), v(v) {}
  glm::vec3 get_point(float t) const { return v * t; }

  glm::vec3 p;
  glm::vec3 v;
};
/**
 * @brief Segment3D class
 *
 */
struct Segment3 : public Line3 {
  Segment3();
  Segment3(const Line3 &l) : Line3(l) {}
  Segment3(const glm::vec3 &p, const glm::vec3 &v);
  /**
   * @brief Get end point of segment
   *
   */
  glm::vec3 get_end_point() const;
};
/**
 * @brief Ray3D class
 *
 */
struct Ray3 : public Line3 {
  Ray3();
  Ray3(const Line3 &l) : Line3(l) {}
  Ray3(const Segment3 &s) : Line3(s) {}
  Ray3(const glm::vec3 &p, const glm::vec3 &v);
  /**
   * @brief Get end point of ray
   *
   */
  glm::vec3 get_end_point() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle3 {
  Triangle3();
  Triangle3(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  glm::vec3 get_normal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float get_area() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  glm::vec3 get_centroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  glm::vec3 get_barycentric(const glm::vec3 &p) const;

  // vertices
  glm::vec3 a;
  glm::vec3 b;
  glm::vec3 c;
};
/**
 * @brief Plane3D class
 *
 */
struct Plane3 {
  Plane3() = default;
  Plane3(const glm::vec3 &p, const glm::vec3 &n) : p(p), n(n) {}

  // point on plane
  glm::vec3 p;
  // normal of plane
  glm::vec3 n;
};
/**
 * @brief Sphere class
 *
 */
struct Sphere {
  Sphere() : r(0.f) {};
  ~Sphere() = default;
  Sphere(const glm::vec3 &p, float r) : p(p), r(r) {}

  // position
  glm::vec3 p;
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
  Capsule(const glm::vec3 &p1, const glm::vec3 &p2, float r)
      : s(p1, p2), r(r) {}
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
