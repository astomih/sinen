#ifndef SINEN_VECTOR4_HPP
#define SINEN_VECTOR4_HPP
namespace sinen {
class Vec3;
class Vec4 {
public:
  float x, y, z, w;
  Vec4() = default;
  Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  Vec4(const Vec3 &v, float w);
  Vec4(float x, const Vec3 &v);
  constexpr explicit Vec4(float v) : x(v), y(v), z(v), w(v) {}
  // Vector addition (a + b)
  friend Vec4 operator+(const Vec4 &a, const Vec4 &b) {
    return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  }

  // Vector subtraction (a - b)
  friend Vec4 operator-(const Vec4 &a, const Vec4 &b) {
    return Vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  }

  // Component-wise multiplication
  friend Vec4 operator*(const Vec4 &left, const Vec4 &right) {
    return Vec4(left.x * right.x, left.y * right.y, left.z * right.z,
                left.w * right.z);
  }

  // Scalar multiplication
  friend Vec4 operator*(const Vec4 &vec, float scalar) {
    return Vec4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
  }

  // Scalar multiplication
  friend Vec4 operator*(float scalar, const Vec4 &vec) {
    return Vec4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
  }

  // Scalar *=
  Vec4 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  // Vector +=
  Vec4 &operator+=(const Vec4 &right) {
    x += right.x;
    y += right.y;
    z += right.z;
    w += right.w;
    return *this;
  }

  // Vector -=
  Vec4 &operator-=(const Vec4 &right) {
    x -= right.x;
    y -= right.y;
    z -= right.z;
    w -= right.w;
    return *this;
  }
  Vec4 &operator/=(float v) {
    x /= v;
    y /= v;
    z /= v;
    w /= v;
    return *this;
  }
  Vec4 &operator=(float v) {
    *this = Vec4(v);
    return *this;
  }

  float &operator[](int index) {
    float *v = &x;
    v += index;
    return *v;
  }
};
} // namespace sinen
#endif