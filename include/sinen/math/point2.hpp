#ifndef SINEN_POINT2_HPP
#define SINEN_POINT2_HPP
namespace sinen {
/**
 * @brief 2D point class
 *
 */
template <typename T> struct point2 {
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
  point2(const T &x, const T &y) : x(x), y(y) {}
  /**
   * @brief Construct a new point2 object
   *
   */
  point2() : x(0), y(0) {}
  ~point2() = default;
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
  float distance(const point2 &p) {
    return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
  }
  point2 operator+(const point2 &p) { return point2(x + p.x, y + p.y); }
  point2 &operator+=(const point2 &p) {
    x += p.x;
    y += p.y;
    return *this;
  }
  point2 operator-(const point2 &p) { return point2(x - p.x, y - p.y); }
  point2 &operator-=(const point2 &p) {
    x -= p.x;
    y -= p.y;
    return *this;
  }
};
using point2i = point2<int>;
using point2f = point2<float>;
using point2d = point2<double>;
} // namespace sinen
#endif