#ifndef SINEN_PRIMITIVE2_HPP
#define SINEN_PRIMITIVE2_HPP
#include "../math/point2.hpp"
#include "../math/vector2.hpp"

namespace sinen {
/**
 * @brief Line2D class
 *
 */
struct Line2 {
  Line2() = default;
  Line2(const Vector2 &p, const Vector2 &v) : p(p), v(v) {}
  Vector2 get_point(float t) const { return v * t; }

  Vector2 p;
  Vector2 v;
};
/**
 * @brief Segment2D class
 *
 */
struct Segment2 : public Line2 {
  Segment2();
  Segment2(const Line2 &l) : Line2(l) {}
  Segment2(const Vector2 &p, const Vector2 &v);
  /**
   * @brief Get end point of segment
   *
   */
  Point2f get_end_point() const;
};
/**
 * @brief Ray2D class
 *
 */
struct Ray2 : public Line2 {
  Ray2();
  Ray2(const Line2 &l) : Line2(l) {}
  Ray2(const Segment2 &s) : Line2(s) {}
  Ray2(const Vector2 &p, const Vector2 &v);
  /**
   * @brief Get end point of ray
   *
   */
  Point2f get_end_point() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle2 {
  Triangle2();
  Triangle2(const Vector2 &a, const Vector2 &b, const Vector2 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  Vector2 get_normal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float get_area() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  Point2f get_centroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  Vector2 get_barycentric(const Vector2 &p) const;

  // vertices
  Vector2 a;
  Vector2 b;
  Vector2 c;
};
/**
 * @brief Plane2D class
 *
 */
struct Plane2 {
  Plane2() = default;
  Plane2(const Vector2 &p, const Vector2 &n) : p(p), n(n) {}

  // point on plane
  Vector2 p;
  // normal of plane
  Vector2 n;
};
/**
 * @brief Rectangle class
 *
 */
struct Rectangle {
  Rectangle() = default;
  Rectangle(const Vector2 &p, const Vector2 &s) : p(p), s(s) {}
  // point on rectangle
  Vector2 p;
  // size of rectangle
  Vector2 s;
};
/**
 * @brief Circle class
 *
 */
struct Circle {
  Circle() = default;
  Circle(const Vector2 &p, float r) : p(p), r(r) {}
  // point on circle
  Vector2 p;
  // radius of circle
  float r;
};

} // namespace sinen

#endif // !SINEN_PRIMITIVE2_HPP