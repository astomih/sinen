#ifndef SINEN_PRIMITIVE2_HPP
#define SINEN_PRIMITIVE2_HPP
#include <math/vector.hpp>
namespace sinen {
/**
 * @brief Line2D class
 *
 */
struct Line2 {
  Line2() = default;
  Line2(const Vec2 &p, const Vec2 &v) : p(p), v(v) {}
  Vec2 getPoint(float t) const { return v * t; }

  Vec2 p;
  Vec2 v;
};
/**
 * @brief Segment2D class
 *
 */
struct Segment2 : public Line2 {
  Segment2();
  Segment2(const Line2 &l) : Line2(l) {}
  Segment2(const Vec2 &p, const Vec2 &v);
  /**
   * @brief Get end point of segment
   *
   */
  Vec2 getEndPoint() const;
};
/**
 * @brief Ray2D class
 *
 */
struct Ray2 : public Line2 {
  Ray2();
  Ray2(const Line2 &l) : Line2(l) {}
  Ray2(const Segment2 &s) : Line2(s) {}
  Ray2(const Vec2 &p, const Vec2 &v);
  /**
   * @brief Get end point of ray
   *
   */
  Vec2 getEndPoint() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle2 {
  Triangle2();
  Triangle2(const Vec2 &a, const Vec2 &b, const Vec2 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  Vec2 getNormal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float getArea() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  Vec2 getCentroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  Vec2 getBarycentric(const Vec2 &p) const;

  // vertices
  Vec2 a;
  Vec2 b;
  Vec2 c;
};
/**
 * @brief Plane2D class
 *
 */
struct Plane2 {
  Plane2() = default;
  Plane2(const Vec2 &p, const Vec2 &n) : p(p), n(n) {}

  // point on plane
  Vec2 p;
  // normal of plane
  Vec2 n;
};
/**
 * @brief Circle class
 *
 */
struct Circle {
  Circle() = default;
  Circle(const Vec2 &p, float r) : p(p), r(r) {}
  // point on circle
  Vec2 p;
  // radius of circle
  float r;
};

} // namespace sinen

#endif // !SINEN_PRIMITIVE2_HPP