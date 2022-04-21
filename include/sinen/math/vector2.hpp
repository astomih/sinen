#pragma once
#include "math.hpp"

namespace nen {
class vector2 {
public:
  float x;
  float y;

  constexpr vector2() : x(0.0f), y(0.0f) {}

  constexpr explicit vector2(float inX, float inY) : x(inX), y(inY) {}

  // Set both components in one line
  void Set(float inX, float inY) {
    x = inX;
    y = inY;
  }

  const float *GetAsFloatPtr() const {
    return reinterpret_cast<const float *>(&x);
  }
  // Vector addition (a + b)
  friend vector2 operator+(const vector2 &a, const vector2 &b) {
    return vector2(a.x + b.x, a.y + b.y);
  }

  // Vector subtraction (a - b)
  friend vector2 operator-(const vector2 &a, const vector2 &b) {
    return vector2(a.x - b.x, a.y - b.y);
  }

  // Component-wise multiplication
  // (a.x * b.x, ...)
  friend vector2 operator*(const vector2 &a, const vector2 &b) {
    return vector2(a.x * b.x, a.y * b.y);
  }

  // Scalar multiplication
  friend vector2 operator*(const vector2 &vec, float scalar) {
    return vector2(vec.x * scalar, vec.y * scalar);
  }

  // Scalar multiplication
  friend vector2 operator*(float scalar, const vector2 &vec) {
    return vector2(vec.x * scalar, vec.y * scalar);
  }

  vector2 &operator/=(float scalar) {

    x > 0.f ? x /= scalar : x = 0.f;
    y > 0.f ? y /= scalar : y = 0.f;
    return *this;
  }

  // Scalar *=
  vector2 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  // Vector +=
  vector2 &operator+=(const vector2 &right) {
    x += right.x;
    y += right.y;
    return *this;
  }

  // Vector -=
  vector2 &operator-=(const vector2 &right) {
    x -= right.x;
    y -= right.y;
    return *this;
  }

  vector2 add(const vector2 &right) const {
    return vector2(x + right.x, y + right.y);
  }
  vector2 sub(const vector2 &right) const {
    return vector2(x - right.x, y - right.y);
  }
  vector2 mul(const vector2 &right) const {
    return vector2(x * right.x, y * right.y);
  }
  vector2 div(const vector2 &right) const {
    return vector2(x / right.x, y / right.y);
  }

  // Length squared of vector
  [[nodiscard]] float LengthSq() const { return (x * x + y * y); }

  // Length of vector
  [[nodiscard]] float Length() const { return (Math::Sqrt(LengthSq())); }

  // Normalize this vector
  void Normalize() {
    const auto length = Length();
    x /= length;
    y /= length;
  }

  // Normalize the provided vector
  static vector2 Normalize(const vector2 &vec) {
    auto temp = vec;
    temp.Normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float Dot(const vector2 &a, const vector2 &b) {
    return (a.x * b.x + a.y * b.y);
  }

  // Lerp from A to B by f
  static vector2 Lerp(const vector2 &a, const vector2 &b, float f) {
    return vector2(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static vector2 Reflect(const vector2 &v, const vector2 &n) {
    return v - 2.0f * vector2::Dot(v, n) * n;
  }

  // Transform vector by matrix
  static vector2 Transform(const vector2 &vec, const class matrix3 &mat,
                           float w = 1.0f);

  static const vector2 Zero;
  static const vector2 UnitX;
  static const vector2 UnitY;
  static const vector2 NegUnitX;
  static const vector2 NegUnitY;
};
} // namespace nen