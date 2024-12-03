#include <math/math.hpp>
#include <physics/collision.hpp>
namespace sinen {
bool Collision::aabb_aabb(const AABB &a, const AABB &b) {
  return (a._min.x <= b._max.x && a._max.x >= b._min.x) &&
         (a._min.y <= b._max.y && a._max.y >= b._min.y) &&
         (a._min.z <= b._max.z && a._max.z >= b._min.z);
}
bool Collision::obb_obb(const OBB &a, const OBB &b) {
  Vector3 nae1 = a.v[0], ae1 = nae1 * a.a[0];
  Vector3 nae2 = a.v[1], ae2 = nae2 * a.a[1];
  Vector3 nae3 = a.v[2], ae3 = nae3 * a.a[2];
  Vector3 nbe1 = b.v[0], be1 = nbe1 * b.a[0];
  Vector3 nbe2 = b.v[1], be2 = nbe2 * b.a[1];
  Vector3 nbe3 = b.v[2], be3 = nbe3 * b.a[2];
  Vector3 interval = a.p - b.p;
  // Ae1
  float rA = ae1.length();
  float rB = segment_length_on_separate_axis(nae1, be1, be2, be3);
  float L = Math::abs(Vector3::dot(interval, nae1));
  if (L > rA + rB)
    return false;

  // Ae2
  rA = ae2.length();
  rB = segment_length_on_separate_axis(nae2, be1, be2, be3);
  L = Math::abs(Vector3::dot(interval, nae2));
  if (L > rA + rB)
    return false;

  // Ae3
  rA = ae3.length();
  rB = segment_length_on_separate_axis(nae3, be1, be2, be3);
  L = Math::abs(Vector3::dot(interval, nae3));
  if (L > rA + rB)
    return false;

  // Be1
  rA = segment_length_on_separate_axis(nbe1, ae1, ae2, ae3);
  rB = be1.length();
  L = Math::abs(Vector3::dot(interval, nbe1));
  if (L > rA + rB)
    return false;

  // Be2
  rA = segment_length_on_separate_axis(nbe2, ae1, ae2, ae3);
  rB = be2.length();
  L = Math::abs(Vector3::dot(interval, nbe2));
  if (L > rA + rB)
    return false;

  // Be3
  rA = segment_length_on_separate_axis(nbe3, ae1, ae2, ae3);
  rB = be3.length();
  L = Math::abs(Vector3::dot(interval, nbe3));
  if (L > rA + rB)
    return false;

  Vector3 cross;
  // C11
  cross = Vector3::cross(nae1, nbe1);
  rA = segment_length_on_separate_axis(cross, ae2, ae3);
  rB = segment_length_on_separate_axis(cross, be2, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C12
  cross = Vector3::cross(nae1, nbe2);
  rA = segment_length_on_separate_axis(cross, ae2, ae3);
  rB = segment_length_on_separate_axis(cross, be1, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C13
  cross = Vector3::cross(nae1, nbe3);
  rA = segment_length_on_separate_axis(cross, ae2, ae3);
  rB = segment_length_on_separate_axis(cross, be1, be2);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C21
  cross = Vector3::cross(nae2, nbe1);
  rA = segment_length_on_separate_axis(cross, ae1, ae3);
  rB = segment_length_on_separate_axis(cross, be2, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C22
  cross = Vector3::cross(nae2, nbe2);
  rA = segment_length_on_separate_axis(cross, ae1, ae3);
  rB = segment_length_on_separate_axis(cross, be1, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C23
  cross = Vector3::cross(nae2, nbe3);
  rA = segment_length_on_separate_axis(cross, ae1, ae3);
  rB = segment_length_on_separate_axis(cross, be1, be2);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C31
  cross = Vector3::cross(nae3, nbe1);
  rA = segment_length_on_separate_axis(cross, ae1, ae2);
  rB = segment_length_on_separate_axis(cross, be2, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C32
  cross = Vector3::cross(nae3, nbe2);
  rA = segment_length_on_separate_axis(cross, ae1, ae2);
  rB = segment_length_on_separate_axis(cross, be1, be3);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // C33
  cross = Vector3::cross(nae3, nbe3);
  rA = segment_length_on_separate_axis(cross, ae1, ae2);
  rB = segment_length_on_separate_axis(cross, be1, be2);
  L = Math::abs(Vector3::dot(interval, cross));
  if (L > rA + rB)
    return false;

  // no separating hyperplane theorem found, so the two OBBs are intersecting
  return true;
}
float Collision::segment_length_on_separate_axis(const Vector3 &Sep,
                                                 const Vector3 &e1,
                                                 const Vector3 &e2,
                                                 const Vector3 &e3) {
  float r1 = Math::abs(Vector3::dot(Sep, e1));
  float r2 = Math::abs(Vector3::dot(Sep, e2));
  float r3 = Math::abs(Vector3::dot(Sep, e3));
  return r1 + r2 + r3;
}
float Collision::segment_length_on_separate_axis(const Vector3 &Sep,
                                                 const Vector3 &e1,
                                                 const Vector3 &e2) {
  float r1 = Math::abs(Vector3::dot(Sep, e1));
  float r2 = Math::abs(Vector3::dot(Sep, e2));
  return r1 + r2;
}
} // namespace sinen
