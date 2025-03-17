#ifndef SINEN_VECTOR4_HPP
#define SINEN_VECTOR4_HPP
namespace sinen {
/**
 * @brief Vector4 class
 *
 */
class Vector4 {
public:
  float x;
  float y;
  float z;
  float w;

  constexpr Vector4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
  constexpr explicit Vector4(const float value)
      : x(value), y(value), z(value), w(value) {}

  constexpr explicit Vector4(const float x, const float y, const float z,
                             const float w)
      : x(x), y(y), z(z), w(w) {}

  // Cast to a const float pointer
  [[nodiscard]] const float *get_ptr() const {
    return reinterpret_cast<const float *>(&x);
  }

  // Set all four components in one line
  void set(float inX, float inY, float inZ, float inW) {
    x = inX;
    y = inY;
    z = inZ;
    w = inW;
  }

  // Vector addition (a + b)
  friend Vector4 operator+(const Vector4 &a, const Vector4 &b) {
    return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  }

  // Vector subtraction (a - b)
  friend Vector4 operator-(const Vector4 &a, const Vector4 &b) {
    return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  }

  // Component-wise multiplication
  friend Vector4 operator*(const Vector4 &left, const Vector4 &right) {
    return Vector4(left.x * right.x, left.y * right.y, left.z * right.z,
                   left.w * right.w);
  }

  // Scalar multiplication
  friend Vector4 operator*(const Vector4 &vec, float scalar) {
    return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar,
                   vec.w * scalar);
  }

  // Scalar multiplication
  friend Vector4 operator*(float scalar, const Vector4 &vec) {
    return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar,
                   vec.w * scalar);
  }

  // Scalar *=
  Vector4 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  // Vector +=
  Vector4 &operator+=(const Vector4 &right) {
    x += right.x;
    y += right.y;
    z += right.z;
    w += right.w;
    return *this;
  }

  // Vector -=
  Vector4 &operator-=(const Vector4 &right) {
    x -= right.x;
    y -= right.y;
    z -= right.z;
    w -= right.w;
    return *this;
  }

  Vector4 add(const Vector4 &right) const {
    return Vector4(x + right.x, y + right.y, z + right.z, w + right.w);
  }
  Vector4 sub(const Vector4 &right) const {
    return Vector4(x - right.x, y - right.y, z - right.z, w - right.w);
  }
  Vector4 mul(const Vector4 &right) const {
    return Vector4(x * right.x, y * right.y, z * right.z, w * right.w);
  }
  Vector4 div(const Vector4 &right) const {
    return Vector4(x / right.x, y / right.y, z / right.z, w / right.w);
  }
};
} // namespace sinen
#endif // !SINEN_VECTOR4_HPP