#include <math/math.hpp>
#include <physics/collision.hpp>

namespace sinen {
void AABB::updateWorld(const Vec3 &p, const Vec3 &scale, const AABB &local) {
  this->min = p + scale * local.min;
  this->max = p + scale * local.max;
}
bool Collision::aabBvsAabb(const AABB &a, const AABB &b) {
  return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
         (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
         (a.min.z <= b.max.z && a.max.z >= b.min.z);
}
bool Collision::obBvsObb(const OBB &a, const OBB &b) {
  Vec3 nae1 = a.v[0], ae1 = nae1 * a.a[0];
  Vec3 nae2 = a.v[1], ae2 = nae2 * a.a[1];
  Vec3 nae3 = a.v[2], ae3 = nae3 * a.a[2];
  Vec3 nbe1 = b.v[0], be1 = nbe1 * b.a[0];
  Vec3 nbe2 = b.v[1], be2 = nbe2 * b.a[1];
  Vec3 nbe3 = b.v[2], be3 = nbe3 * b.a[2];
  Vec3 interval = a.p - b.p;
  // Ae1
  float rA = ae1.length();
  float rB = segmentLengthOnSeparateAxis(nae1, be1, be2, be3);
  float L = Math::abs(Vec3::dot(interval, nae1));
  if (L > rA + rB)
    return false;

  // Ae2
  rA = ae2.length();
  rB = segmentLengthOnSeparateAxis(nae2, be1, be2, be3);
  L = Math::abs(Vec3::dot(interval, nae2));
  if (L > rA + rB)
    return false;

  // Ae3
  rA = ae3.length();
  rB = segmentLengthOnSeparateAxis(nae3, be1, be2, be3);
  L = Math::abs(Vec3::dot(interval, nae3));
  if (L > rA + rB)
    return false;

  // Be1
  rA = segmentLengthOnSeparateAxis(nbe1, ae1, ae2, ae3);
  rB = be1.length();
  L = Math::abs(Vec3::dot(interval, nbe1));
  if (L > rA + rB)
    return false;

  // Be2
  rA = segmentLengthOnSeparateAxis(nbe2, ae1, ae2, ae3);
  rB = be2.length();
  L = Math::abs(Vec3::dot(interval, nbe2));
  if (L > rA + rB)
    return false;

  // Be3
  rA = segmentLengthOnSeparateAxis(nbe3, ae1, ae2, ae3);
  rB = be3.length();
  L = Math::abs(Vec3::dot(interval, nbe3));
  if (L > rA + rB)
    return false;

  Vec3 cross;
  // C11
  cross = Vec3::cross(nae1, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C12
  cross = Vec3::cross(nae1, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C13
  cross = Vec3::cross(nae1, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae2, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C21
  cross = Vec3::cross(nae2, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C22
  cross = Vec3::cross(nae2, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C23
  cross = Vec3::cross(nae2, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae3);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C31
  cross = Vec3::cross(nae3, nbe1);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be2, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C32
  cross = Vec3::cross(nae3, nbe2);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be1, be3);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C33
  cross = Vec3::cross(nae3, nbe3);
  rA = segmentLengthOnSeparateAxis(cross, ae1, ae2);
  rB = segmentLengthOnSeparateAxis(cross, be1, be2);
  L = Math::abs(Vec3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // no separating hyperplane theorem found, so the two OBBs are intersecting
  return true;
}
float Collision::segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                             const Vec3 &e2, const Vec3 &e3) {
  float r1 = Math::abs(Vec3::dot(Sep, e1));
  float r2 = Math::abs(Vec3::dot(Sep, e2));
  float r3 = Math::abs(Vec3::dot(Sep, e3));
  return r1 + r2 + r3;
}
float Collision::segmentLengthOnSeparateAxis(const Vec3 &Sep, const Vec3 &e1,
                                             const Vec3 &e2) {
  float r1 = Math::abs(Vec3::dot(Sep, e1));
  float r2 = Math::abs(Vec3::dot(Sep, e2));
  return r1 + r2;
}
} // namespace sinen
