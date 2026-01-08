#ifndef SINEN_VECTOR2_HPP
#define SINEN_VECTOR2_HPP
#include "math.hpp"
namespace sinen {
class Vec2 {
public:
  float x;
  float y;
  /**
   * @brief Construct a new Vec2 object
   *
   */
  constexpr Vec2() : x(0.0f), y(0.0f) {}

  constexpr explicit Vec2(float v) : x(v), y(v) {}
  /**
   * @brief Construct a new Vec2 object
   *
   */
  constexpr Vec2(float inX, float inY) : x(inX), y(inY) {}

  // Set both components in one line
  void Set(float inX, float inY) {
    x = inX;
    y = inY;
  }

  const float *get_ptr() const { return reinterpret_cast<const float *>(&x); }
  // Vector addition (a + b)
  friend Vec2 operator+(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x + b.x, a.y + b.y);
  }

  // Vector subtraction (a - b)
  friend Vec2 operator-(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x - b.x, a.y - b.y);
  }

  // Component-wise multiplication
  // (a.x * b.x, ...)
  friend Vec2 operator*(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x * b.x, a.y * b.y);
  }

  // Scalar multiplication
  friend Vec2 operator*(const Vec2 &vec, float scalar) {
    return Vec2(vec.x * scalar, vec.y * scalar);
  }

  // Scalar multiplication
  friend Vec2 operator*(float scalar, const Vec2 &vec) {
    return Vec2(vec.x * scalar, vec.y * scalar);
  }

  friend Vec2 operator/(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x / b.x, a.y / b.y);
  }

  Vec2 &operator/=(float scalar) {

    x > 0.f ? x /= scalar : x = 0.f;
    y > 0.f ? y /= scalar : y = 0.f;
    return *this;
  }

  // Scalar *=
  Vec2 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  // Vector +=
  Vec2 &operator+=(const Vec2 &right) {
    x += right.x;
    y += right.y;
    return *this;
  }

  // Vector -=
  Vec2 &operator-=(const Vec2 &right) {
    x -= right.x;
    y -= right.y;
    return *this;
  }

  Vec2 add(const Vec2 &right) const { return Vec2(x + right.x, y + right.y); }
  Vec2 sub(const Vec2 &right) const { return Vec2(x - right.x, y - right.y); }
  Vec2 mul(const Vec2 &right) const { return Vec2(x * right.x, y * right.y); }
  Vec2 div(const Vec2 &right) const { return Vec2(x / right.x, y / right.y); }

  // Length squared of vector
  [[nodiscard]] float length_sqrt() const { return (x * x + y * y); }

  // Length of vector
  [[nodiscard]] float length() const { return (Math::sqrt(length_sqrt())); }

  // Normalize this vector
  void normalize() {
    const auto len = length();
    x /= len;
    y /= len;
  }

  // Normalize the provided vector
  static Vec2 normalize(const Vec2 &vec) {
    auto temp = vec;
    temp.normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float dot(const Vec2 &a, const Vec2 &b) {
    return (a.x * b.x + a.y * b.y);
  }

  // Lerp from A to B by f
  static Vec2 lerp(const Vec2 &a, const Vec2 &b, float f) {
    return Vec2(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static Vec2 reflect(const Vec2 &v, const Vec2 &n) {
    return v - 2.0f * Vec2::dot(v, n) * n;
  }

  // Transform vector by matrix
  static Vec2 transform(const Vec2 &vec, const class matrix3 &mat,
                        float w = 1.0f);

  static const Vec2 zero;
  static const Vec2 unit_x;
  static const Vec2 unit_y;
  static const Vec2 neg_unit_x;
  static const Vec2 neg_unit_y;
};
} // namespace sinen
#endif