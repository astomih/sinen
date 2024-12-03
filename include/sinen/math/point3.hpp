#ifndef SINEN_POINT3_HPP
#define SINEN_POINT3_HPP
#include "math.hpp"
namespace sinen {
/**
 * @brief 2D coordinate class
 *
 */
template <typename T> struct Point3 {
  /**
   * @brief x coordinate
   *
   */
  T x;
  /**
   * @brief y coordinate
   *
   */
  T y;
  /**
   * @brief z coordinate
   *
   */
  T z;
  /**
   * @brief Construct a new point3 object
   *
   * @param x x coordinate
   * @param y y coordinate
   * @param z z coordinate
   */
  Point3(const T &x, const T &y, const T &z) : x(x), y(y), z(z) {}
  /**
   * @brief Construct a new point3 object
   *
   */
  Point3() : x(0), y(0), z(0) {}
  /**
   * @brief Construct a new point3 object
   *
   * @param p lvalue point3 object
   */
  Point3(const Point3 &p) = default;
  /**
   * @brief Construct a new point3 object
   *
   * @param p rvalue reference
   */
  Point3(Point3 &&p) = default;
  /**
   * @brief Destroy the point3 object
   *
   */
  ~Point3() = default;
  /**
   * @brief Length of the point3 object
   *
   * @return float Length
   */
  float length() { return sqrt(x * x + y * y + z * z); }
  /**
   * @brief Distance between two point3 objects
   *
   * @param p The other point3 object
   * @return float Distance
   */
  float distance(const Point3 &p) {
    return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y) +
                (z - p.z) * (z - p.z));
  }
  Point3 operator+(const Point3 &p) {
    return point3(x + p.x, y + p.y, z + p.z);
  }
  Point3 &operator+=(const Point3 &p) {
    x += p.x;
    y += p.y;
    z += p.z;
    return *this;
  }
  Point3 operator-(const Point3 &p) {
    return point3(x - p.x, y - p.y, z - p.z);
  }
  Point3 &operator-=(const Point3 &p) {
    x -= p.x;
    y -= p.y;
    z -= p.z;
    return *this;
  }
};
using Point3i = Point3<int>;
using Point3f = Point3<float>;
using Point3d = Point3<double>;
} // namespace sinen
#endif