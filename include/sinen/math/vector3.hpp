#ifndef SINEN_VECTOR3_HPP
#define SINEN_VECTOR3_HPP
#include "math.hpp"
namespace sinen {
/**
 * @brief Vector3 class
 *
 */
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
  [[nodiscard]] const float *get_ptr() const {
    return reinterpret_cast<const float *>(&x);
  }

  // Set all three components in one line
  void set(float inX, float inY, float inZ) {
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

  vector3 add(const vector3 &right) const {
    return vector3(x + right.x, y + right.y, z + right.z);
  }
  vector3 sub(const vector3 &right) const {
    return vector3(x - right.x, y - right.y, z - right.z);
  }
  vector3 mul(const vector3 &right) const {
    return vector3(x * right.x, y * right.y, z * right.z);
  }
  vector3 div(const vector3 &right) const {
    return vector3(x / right.x, y / right.y, z / right.z);
  }
  vector3 copy() { return vector3(x, y, z); }

  // Length squared of vector
  [[nodiscard]] float length_sqrt() const { return (x * x + y * y + z * z); }

  // Length of vector
  [[nodiscard]] float length() const { return (Math::sqrt(length_sqrt())); }

  // Normalize this vector
  void normalize() {
    const auto len = length();
    x /= len;
    y /= len;
    z /= len;
  }

  // Normalize the provided vector
  static vector3 normalize(const vector3 &vec) {
    auto temp = vec;
    temp.normalize();
    return temp;
  }

  // Dot product between two vectors (a dot b)
  static float dot(const vector3 &a, const vector3 &b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
  }

  // Cross product between two vectors (a cross b)
  static vector3 cross(const vector3 &a, const vector3 &b) {
    vector3 temp;
    temp.x = a.y * b.z - a.z * b.y;
    temp.y = a.z * b.x - a.x * b.z;
    temp.z = a.x * b.y - a.y * b.x;
    return temp;
  }

  // Lerp from A to B by f
  static vector3 lerp(const vector3 &a, const vector3 &b, float f) {
    return vector3(a + f * (b - a));
  }

  // Reflect V about (normalized) N
  static vector3 reflect(const vector3 &v, const vector3 &n) {
    return v - 2.0f * vector3::dot(v, n) * n;
  }

  static vector3 transform(const vector3 &vec, const class matrix4 &mat,
                           float w = 1.0f);
  // This will transform the vector and renormalize the w component
  static vector3 transform_with_persp_div(const vector3 &vec,
                                          const class matrix4 &mat,
                                          float w = 1.0f);

  // Transform a Vector3f by a quaternion
  static vector3 transform(const vector3 &v, const class quaternion &q);

  static const vector3 zero;
  static const vector3 unit_x;
  static const vector3 unit_y;
  static const vector3 unit_z;
  static const vector3 neg_unit_x;
  static const vector3 neg_unit_y;
  static const vector3 neg_unit_z;
  static const vector3 infinity;
  static const vector3 neg_infinity;
};
} // namespace sinen
#endif // !SINEN_VECTOR3_HPP