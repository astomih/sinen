#ifndef SINEN_VECTOR2_HPP
#define SINEN_VECTOR2_HPP
#include "math.hpp"

namespace sinen {
/**
 * @brief Vector2 class
 *
 */
class Vector2 {
public:
  float x;
  float y;
  /**
   * @brief Construct a new vector2 object
   *
   */
  constexpr Vector2() : x(0.0f), y(0.0f) {}
  /**
   * @brief Construct a new vector2 object
   *
   */
  constexpr explicit Vector2(float inX, float inY) : x(inX), y(inY) {}

  // Set both components in one line
  void Set(float inX, float inY) {
    x = inX;
    y = inY;
  }

  const float *get_ptr() const { return reinterpret_cast<const float *>(&x); }
  // Vector addition (a + b)
  friend Vector2 operator+(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.x + b.x, a.y + b.y);
  }

  // Vector subtraction (a - b)
  friend Vector2 operator-(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.x - b.x, a.y - b.y);
  }

  // Component-wise multiplication
  // (a.x * b.x, ...)
  friend Vector2 operator*(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.x * b.x, a.y * b.y);
  }

  // Scalar multiplication
  friend Vector2 operator*(const Vector2 &vec, float scalar) {
    return Vector2(vec.x * scalar, vec.y * scalar);
  }

  // Scalar multiplication
  friend Vector2 operator*(float scalar, const Vector2 &vec) {
    return Vector2(vec.x * scalar, vec.y * scalar);
  }

  Vector2 &operator/=(float scalar) {

    x > 0.f ? x /= scalar : x = 0.f;
    y > 0.f ? y /= scalar : y = 0.f;
    return *this;
  }

  // Scalar *=
  Vector2 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  // Vector +=
  Vector2 &operator+=(const Vector2 &right) {
    x += right.x;
    y += right.y;
    return *this;
  }

  // Vector -=
  Vector2 &operator-=(const Vector2 &right) {
    x -= right.x;
    y -= right.y;
    return *this;
  }

  Vector2 add(const Vector2 &right) const {
    return Vector2(x + right.x, y + right.y);
  }
  Vector2 sub(const Vector2 &right) const {
    return Vector2(x - right.x, y - right.y);
  }
  Vector2 mul(const Vector2 &right) const {
    return Vector2(x * right.x, y * right.y);
  }
  Vector2 div(const Vector2 &right) const {
    return Vector2(x / right.x, y / right.y);
  }

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
  static Vector2 normalize(const Vector2 &vec) {
    auto temp = vec;
    temp.normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float dot(const Vector2 &a, const Vector2 &b) {
    return (a.x * b.x + a.y * b.y);
  }

  // Lerp from A to B by f
  static Vector2 lerp(const Vector2 &a, const Vector2 &b, float f) {
    return Vector2(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static Vector2 reflect(const Vector2 &v, const Vector2 &n) {
    return v - 2.0f * Vector2::dot(v, n) * n;
  }

  // Transform vector by matrix
  static Vector2 transform(const Vector2 &vec, const class Matrix3 &mat,
                           float w = 1.0f);

  static const Vector2 zero;
  static const Vector2 unit_x;
  static const Vector2 unit_y;
  static const Vector2 neg_unit_x;
  static const Vector2 neg_unit_y;
};
} // namespace sinen
#endif // !SINEN_VECTOR2_HPP
