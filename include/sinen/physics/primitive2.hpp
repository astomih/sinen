#ifndef SINEN_PRIMITIVE2_HPP
#define SINEN_PRIMITIVE2_HPP
#include "../math/point2.hpp"
#include "../math/vector2.hpp"

namespace sinen {
/**
 * @brief Line2D class
 *
 */
struct line2 {
  line2() = default;
  line2(const vector2 &p, const vector2 &v) : p(p), v(v) {}
  vector2 get_point(float t) const { return v * t; }

  vector2 p;
  vector2 v;
};
/**
 * @brief Segment2D class
 *
 */
struct segment2 : public line2 {
  segment2();
  segment2(const line2 &l) : line2(l) {}
  segment2(const vector2 &p, const vector2 &v);
  /**
   * @brief Get end point of segment
   *
   */
  point2f get_end_point() const;
};
/**
 * @brief Ray2D class
 *
 */
struct ray2 : public line2 {
  ray2();
  ray2(const line2 &l) : line2(l) {}
  ray2(const segment2 &s) : line2(s) {}
  ray2(const vector2 &p, const vector2 &v);
  /**
   * @brief Get end point of ray
   *
   */
  point2f get_end_point() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct triangle2 {
  triangle2();
  triangle2(const vector2 &a, const vector2 &b, const vector2 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  vector2 get_normal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float get_area() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  point2f get_centroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  vector2 get_barycentric(const vector2 &p) const;

  // vertices
  vector2 a;
  vector2 b;
  vector2 c;
};
/**
 * @brief Plane2D class
 *
 */
struct plane2 {
  plane2() = default;
  plane2(const vector2 &p, const vector2 &n) : p(p), n(n) {}

  // point on plane
  vector2 p;
  // normal of plane
  vector2 n;
};
/**
 * @brief Rectangle class
 *
 */
struct rectangle {
  rectangle() = default;
  rectangle(const vector2 &p, const vector2 &s) : p(p), s(s) {}
  // point on rectangle
  vector2 p;
  // size of rectangle
  vector2 s;
};
/**
 * @brief Circle class
 *
 */
struct circle {
  circle() = default;
  circle(const vector2 &p, float r) : p(p), r(r) {}
  // point on circle
  vector2 p;
  // radius of circle
  float r;
};

} // namespace sinen

#endif // !SINEN_PRIMITIVE2_HPP