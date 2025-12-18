#ifndef SINEN_PRIMITIVE2_HPP
#define SINEN_PRIMITIVE2_HPP
#include <glm/vec2.hpp>
namespace sinen {
/**
 * @brief Line2D class
 *
 */
struct Line2 {
  Line2() = default;
  Line2(const glm::vec2 &p, const glm::vec2 &v) : p(p), v(v) {}
  glm::vec2 getPoint(float t) const { return v * t; }

  glm::vec2 p;
  glm::vec2 v;
};
/**
 * @brief Segment2D class
 *
 */
struct Segment2 : public Line2 {
  Segment2();
  Segment2(const Line2 &l) : Line2(l) {}
  Segment2(const glm::vec2 &p, const glm::vec2 &v);
  /**
   * @brief Get end point of segment
   *
   */
  glm::vec2 getEndPoint() const;
};
/**
 * @brief Ray2D class
 *
 */
struct Ray2 : public Line2 {
  Ray2();
  Ray2(const Line2 &l) : Line2(l) {}
  Ray2(const Segment2 &s) : Line2(s) {}
  Ray2(const glm::vec2 &p, const glm::vec2 &v);
  /**
   * @brief Get end point of ray
   *
   */
  glm::vec2 getEndPoint() const;
};
/**
 * @brief Triangle3D class
 *
 */
struct Triangle2 {
  Triangle2();
  Triangle2(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c);
  /**
   * @brief Get normal of triangle
   *
   */
  glm::vec2 getNormal() const;
  /**
   * @brief Get area of triangle
   *
   */
  float getArea() const;
  /**
   * @brief Get centroid of triangle
   *
   */
  glm::vec2 getCentroid() const;
  /**
   * @brief Get barycentric coordinates of point
   *
   */
  glm::vec2 getBarycentric(const glm::vec2 &p) const;

  // vertices
  glm::vec2 a;
  glm::vec2 b;
  glm::vec2 c;
};
/**
 * @brief Plane2D class
 *
 */
struct Plane2 {
  Plane2() = default;
  Plane2(const glm::vec2 &p, const glm::vec2 &n) : p(p), n(n) {}

  // point on plane
  glm::vec2 p;
  // normal of plane
  glm::vec2 n;
};
/**
 * @brief Rectangle class
 *
 */
struct Rect {
  Rect() = default;
  Rect(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height) {}
  Rect(const glm::vec2 &p, const glm::vec2 &s) : p(p), s(s) {}
  union {
    struct {
      float x;
      float y;
      float width;
      float height;
    };
    struct {
      glm::vec2 p;
      glm::vec2 s;
    };
  };
};
/**
 * @brief Circle class
 *
 */
struct Circle {
  Circle() = default;
  Circle(const glm::vec2 &p, float r) : p(p), r(r) {}
  // point on circle
  glm::vec2 p;
  // radius of circle
  float r;
};

} // namespace sinen

#endif // !SINEN_PRIMITIVE2_HPP