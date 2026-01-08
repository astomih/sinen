#include "quaternion.hpp"
#include <math/vector.hpp>

namespace sinen {
const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);
Quaternion Quaternion::from_euler(const Vec3 &euler) {
  Quaternion q;
  q = Quaternion::concatenate(
      q, Quaternion(Vec3::unit_z, Math::toRadians(euler.z)));
  q = Quaternion::concatenate(
      q, Quaternion(Vec3::unit_y, Math::toRadians(euler.y)));
  q = Quaternion::concatenate(
      q, Quaternion(Vec3::unit_x, Math::toRadians(euler.x)));
  return q;
}

Vec3 Quaternion::to_euler(const Quaternion &r) {
  float x = r.x;
  float y = r.y;
  float z = r.z;
  float w = r.w;

  float x2 = x * x;
  float y2 = y * y;
  float z2 = z * z;

  float xy = x * y;
  float xz = x * z;
  float yz = y * z;
  float wx = w * x;
  float wy = w * y;
  float wz = w * z;

  // 1 - 2y^2 - 2z^2
  float m00 = 1.f - (2.f * y2) - (2.f * z2);

  // 2xy + 2wz
  float m01 = (2.f * xy) + (2.f * wz);

  // 2xy - 2wz
  float m10 = (2.f * xy) - (2.f * wz);

  // 1 - 2x^2 - 2z^2
  float m11 = 1.f - (2.f * x2) - (2.f * z2);

  // 2xz + 2wy
  float m20 = (2.f * xz) + (2.f * wy);

  // 2yz+2wx
  float m21 = (2.f * yz) - (2.f * wx);

  // 1 - 2x^2 - 2y^2
  float m22 = 1.f - (2.f * x2) - (2.f * y2);

  float tx, ty, tz;

  if (m21 >= 0.99 && m21 <= 1.01) {
    tx = Math::pi / 2.f;
    ty = 0;
    tz = Math::atan2(m10, m00);
  } else if (m21 >= -1.01f && m21 <= -0.99f) {
    tx = -Math::pi / 2.f;
    ty = 0;
    tz = Math::atan2(m10, m00);
  } else {
    tx = std::asin(-m21);
    ty = Math::atan2(m20, m22);
    tz = Math::atan2(m01, m11);
  }

  return Vec3(tx, ty, tz);
}
} // namespace sinen