#ifndef SINEN_QUATERNION_HPP
#define SINEN_QUATERNION_HPP
#include "math.hpp"
#include "vector3.hpp"
namespace sinen {
/**
 * @brief Quaternion class
 *
 */
class quaternion {
public:
  float x{};
  float y{};
  float z{};
  float w{};
  /**
   * @brief Construct a new quaternion object
   *
   */
  constexpr quaternion() { *this = quaternion::Identity; }
  /**
   * @brief Construct a new quaternion object
   *
   * @param inX
   * @param inY
   * @param inZ
   * @param inW
   */
  explicit quaternion(float inX, float inY, float inZ, float inW) {
    set(inX, inY, inZ, inW);
  }
  explicit quaternion(const vector3 &axis, float angle) {
    const auto scalar = math::sin(angle / 2.0f);
    x = axis.x * scalar;
    y = axis.y * scalar;
    z = axis.z * scalar;
    w = math::cos(angle / 2.0f);
  }
  /**
   * @brief Directly set the quaternion values
   *
   * @param x x
   * @param y y
   * @param z z
   * @param w w
   */
  void set(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }
  void conjugate() {
    x *= -1.0f;
    y *= -1.0f;
    z *= -1.0f;
  }
  [[nodiscard]] float length_sqrt() const {
    return (x * x + y * y + z * z + w * w);
  }
  [[nodiscard]] float length() const { return math::sqrt(length_sqrt()); }
  void normalize() {
    const auto len = length();
    x /= len;
    y /= len;
    z /= len;
    w /= len;
  }
  // Normalize the provided quaternion
  static quaternion normalize(const quaternion &q) {
    auto retVal = q;
    retVal.normalize();
    return retVal;
  }
  // Linear interpolation
  static quaternion lerp(const quaternion &a, const quaternion &b, float f) {
    quaternion retVal;
    retVal.x = math::lerp(a.x, b.x, f);
    retVal.y = math::lerp(a.y, b.y, f);
    retVal.z = math::lerp(a.z, b.z, f);
    retVal.w = math::lerp(a.w, b.w, f);
    retVal.normalize();
    return retVal;
  }
  static float dot(const quaternion &a, const quaternion &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  }
  // Spherical Linear Interpolation
  static quaternion slerp(const quaternion &a, const quaternion &b, float f) {
    const auto rawCosm = quaternion::dot(a, b);
    auto cosom = -rawCosm;
    if (rawCosm >= 0.0f) {
      cosom = rawCosm;
    }
    float scale0, scale1;
    if (cosom < 0.9999f) {
      const auto omega = math::acos(cosom);
      const auto invSin = 1.f / math::sin(omega);
      scale0 = math::sin((1.f - f) * omega) * invSin;
      scale1 = math::sin(f * omega) * invSin;
    } else {
      scale0 = 1.0f - f;
      scale1 = f;
    }
    if (rawCosm < 0.0f) {
      scale1 = -scale1;
    }
    quaternion retVal;
    retVal.x = scale0 * a.x + scale1 * b.x;
    retVal.y = scale0 * a.y + scale1 * b.y;
    retVal.z = scale0 * a.z + scale1 * b.z;
    retVal.w = scale0 * a.w + scale1 * b.w;
    retVal.normalize();
    return retVal;
  }

  // Concatenate
  // Rotate by q FOLLOWED BY p
  static quaternion concatenate(const quaternion &q, const quaternion &p) {
    quaternion retVal;

    const vector3 qv(q.x, q.y, q.z);
    const vector3 pv(p.x, p.y, p.z);
    const auto newVec = p.w * qv + q.w * pv + vector3::cross(pv, qv);
    retVal.x = newVec.x;
    retVal.y = newVec.y;
    retVal.z = newVec.z;

    retVal.w = p.w * q.w - vector3::dot(pv, qv);

    return retVal;
  }
  /**
   * @brief Quaternion to Euler angles
   *
   * @param r rotation quaternion
   * @return vector3 euler angles
   */
  static vector3 to_euler(const quaternion &r);
  static const quaternion Identity;
};
} // namespace sinen
#endif // !SINEN_QUATERNION_HPP
