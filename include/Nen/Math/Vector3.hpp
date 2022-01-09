#pragma once
#include "Math.hpp"
namespace nen {
// 3D Vector
class vector3 {
public:
  float x;
  float y;
  float z;

  constexpr vector3() : x(0.f), y(0.f), z(0.f) {}
  constexpr explicit vector3(const float value)
      : x(value), y(value), z(value) {}

  constexpr explicit vector3(const float x, const float y, const float z)
      : x(x), y(y), z(z) {}

  // Cast to a const float pointer
  [[nodiscard]] const float *GetAsFloatPtr() const {
    return reinterpret_cast<const float *>(&x);
  }

  // Set all three components in one line
  void Set(float inX, float inY, float inZ) {
    x = inX;
    y = inY;
    z = inZ;
  }

  // Vector addition (a + b)
  friend vector3 operator+(const vector3 &a, const vector3 &b) {
    return vector3(a.x + b.x, a.y + b.y, a.z + b.z);
  }

  // Vector subtraction (a - b)
  friend vector3 operator-(const vector3 &a, const vector3 &b) {
    return vector3(a.x - b.x, a.y - b.y, a.z - b.z);
  }

  // Component-wise multiplication
  friend vector3 operator*(const vector3 &left, const vector3 &right) {
    return vector3(left.x * right.x, left.y * right.y, left.z * right.z);
  }

  // Scalar multiplication
  friend vector3 operator*(const vector3 &vec, float scalar) {
    return vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
  }

  // Scalar multiplication
  friend vector3 operator*(float scalar, const vector3 &vec) {
    return vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
  }

  // Scalar *=
  vector3 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  // Vector +=
  vector3 &operator+=(const vector3 &right) {
    x += right.x;
    y += right.y;
    z += right.z;
    return *this;
  }

  // Vector -=
  vector3 &operator-=(const vector3 &right) {
    x -= right.x;
    y -= right.y;
    z -= right.z;
    return *this;
  }

  // Length squared of vector
  [[nodiscard]] float LengthSq() const { return (x * x + y * y + z * z); }

  // Length of vector
  [[nodiscard]] float Length() const { return (Math::Sqrt(LengthSq())); }

  // Normalize this vector
  void Normalize() {
    const auto length = Length();
    x /= length;
    y /= length;
    z /= length;
  }

  // Normalize the provided vector
  static vector3 Normalize(const vector3 &vec) {
    auto temp = vec;
    temp.Normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float Dot(const vector3 &a, const vector3 &b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
  }

  // Cross product between two vectors (a cross b)
  static vector3 Cross(const vector3 &a, const vector3 &b) {
    vector3 temp;
    temp.x = a.y * b.z - a.z * b.y;
    temp.y = a.z * b.x - a.x * b.z;
    temp.z = a.x * b.y - a.y * b.x;
    return temp;
  }

  // Lerp from A to B by f
  static vector3 Lerp(const vector3 &a, const vector3 &b, float f) {
    return vector3(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static vector3 Reflect(const vector3 &v, const vector3 &n) {
    return v - 2.0f * vector3::Dot(v, n) * n;
  }

  static vector3 Transform(const vector3 &vec, const class matrix4 &mat,
                           float w = 1.0f);
  // This will transform the vector and renormalize the w component
  static vector3 TransformWithPerspDiv(const vector3 &vec,
                                       const class matrix4 &mat,
                                       float w = 1.0f);

  // Transform a Vector3f by a quaternion
  static vector3 Transform(const vector3 &v, const class quaternion &q);

  /// <summary>
  /// VDE Vector3f to Effekseer Vector3D
  /// </summary>
  // Effekseer::Vector3D ToEffekseer() const;

  static const vector3 Zero;
  static const vector3 UnitX;
  static const vector3 UnitY;
  static const vector3 UnitZ;
  static const vector3 NegUnitX;
  static const vector3 NegUnitY;
  static const vector3 NegUnitZ;
  static const vector3 Infinity;
  static const vector3 NegInfinity;
};
} // namespace nen