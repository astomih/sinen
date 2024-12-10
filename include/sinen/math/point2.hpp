#ifndef SINEN_POINT2_HPP
#define SINEN_POINT2_HPP
namespace sinen {
/**
 * @brief 2D point class
 *
 */
template <typename T> struct Point2 {
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
   * @brief Construct a new point2 object
   *
   * @param x x coordinate
   * @param y y coordinate
   */
  Point2(const T &x, const T &y) : x(x), y(y) {}
  /**
   * @brief Construct a new point2 object
   *
   */
  Point2() : x(0), y(0) {}
  ~Point2() = default;
  /**
   * @brief Length of the point2 object
   *
   * @return float Length
   */
  float length() { return sqrt(x * x + y * y); }
  /**
   * @brief Distance between two point2 objects
   *
   * @param p The other point2 object
   * @return float Distance
   */
  float distance(const Point2 &p) {
    return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
  }
  Point2 operator+(const Point2 &p) { return point2(x + p.x, y + p.y); }
  Point2 &operator+=(const Point2 &p) {
    x += p.x;
    y += p.y;
    return *this;
  }
  Point2 operator-(const Point2 &p) { return point2(x - p.x, y - p.y); }
  Point2 &operator-=(const Point2 &p) {
    x -= p.x;
    y -= p.y;
    return *this;
  }
};
using Point2i = Point2<int>;
using Point2f = Point2<float>;
using Point2d = Point2<double>;
} // namespace sinen
#endif