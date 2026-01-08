#ifndef SINEN_VEC3_HPP
#define SINEN_VEC3_HPP
#include "math.hpp"
namespace sinen {
class Vec4;
class Vec3 {
public:
  float x;
  float y;
  float z;

  constexpr Vec3() : x(0.f), y(0.f), z(0.f) {}
  constexpr explicit Vec3(const float value) : x(value), y(value), z(value) {}

  constexpr Vec3(const float x, const float y, const float z)
      : x(x), y(y), z(z) {}

  Vec3(const Vec4 &v);

  // Vector addition (a + b)
  friend Vec3 operator+(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
  }

  // Vector subtraction (a - b)
  friend Vec3 operator-(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
  }

  // Component-wise multiplication
  friend Vec3 operator*(const Vec3 &left, const Vec3 &right) {
    return Vec3(left.x * right.x, left.y * right.y, left.z * right.z);
  }

  // Scalar multiplication
  friend Vec3 operator*(const Vec3 &vec, float scalar) {
    return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
  }

  // Scalar multiplication
  friend Vec3 operator*(float scalar, const Vec3 &vec) {
    return Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
  }

  friend Vec3 operator/(const Vec3 &left, const Vec3 &right) {
    return Vec3(left.x / right.x, left.y / right.y, left.z / right.z);
  }
  // Scalar *=
  Vec3 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  // Vector +=
  Vec3 &operator+=(const Vec3 &right) {
    x += right.x;
    y += right.y;
    z += right.z;
    return *this;
  }

  // Vector -=
  Vec3 &operator-=(const Vec3 &right) {
    x -= right.x;
    y -= right.y;
    z -= right.z;
    return *this;
  }

  Vec3 &operator/=(float v) {
    x /= v;
    y /= v;
    z /= v;
    return *this;
  }

  Vec3 copy() { return Vec3(x, y, z); }

  // Length squared of vector
  [[nodiscard]] float lengthSqrt() const { return (x * x + y * y + z * z); }

  // Length of vector
  [[nodiscard]] float length() const { return (Math::sqrt(lengthSqrt())); }

  // Normalize this vector
  void normalize() {
    const auto len = length();
    x /= len;
    y /= len;
    z /= len;
  }

  // Normalize the provided vector
  static Vec3 normalize(const Vec3 &vec) {
    auto temp = vec;
    temp.normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float dot(const Vec3 &a, const Vec3 &b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
  }

  // Cross product between two vectors (a cross b)
  static Vec3 cross(const Vec3 &a, const Vec3 &b) {
    Vec3 temp;
    temp.x = a.y * b.z - a.z * b.y;
    temp.y = a.z * b.x - a.x * b.z;
    temp.z = a.x * b.y - a.y * b.x;
    return temp;
  }

  // Lerp from A to B by f
  static Vec3 lerp(const Vec3 &a, const Vec3 &b, float f) {
    return Vec3(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static Vec3 reflect(const Vec3 &v, const Vec3 &n) {
    return v - 2.0f * Vec3::dot(v, n) * n;
  }

  static Vec3 transform(const Vec3 &vec, const class Mat4 &mat, float w = 1.0f);
  // This will transform the vector and renormalize the w component
  static Vec3 transformWithPerspDiv(const Vec3 &vec, const class Mat4 &mat,
                                    float w = 1.0f);

  // Transform a Vec3f by a quaternion
  static Vec3 transform(const Vec3 &v, const class Quat &q);
};
} // namespace sinen
#endif