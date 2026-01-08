#include "vec3.hpp"
#include "mat4.hpp"
#include "vec4.hpp"

namespace sinen {
Vec3::Vec3(const Vec4 &v) : x(v.x), y(v.y), z(v.z) {}

Vec3 Vec3::transform(const Vec3 &vec, const Mat4 &mat, float w /*= 1.0f*/) {
  Vec3 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
             vec.z * mat.mat[2][0] + w * mat.mat[3][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
             vec.z * mat.mat[2][1] + w * mat.mat[3][1];
  retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
             vec.z * mat.mat[2][2] + w * mat.mat[3][2];
  // ignore w since we aren't returning a new value for it...
  return retVal;
}

// This will transform the vector and renormalize the w component
Vec3 Vec3::transformWithPerspDiv(const Vec3 &vec, const Mat4 &mat,
                                 float w /*= 1.0f*/) {
  Vec3 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
             vec.z * mat.mat[2][0] + w * mat.mat[3][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
             vec.z * mat.mat[2][1] + w * mat.mat[3][1];
  retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
             vec.z * mat.mat[2][2] + w * mat.mat[3][2];
  float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
                       vec.z * mat.mat[2][3] + w * mat.mat[3][3];
  if (!Math::nearZero(Math::abs(transformedW))) {
    transformedW = 1.0f / transformedW;
    retVal *= transformedW;
  }
  return retVal;
}

// Transform a Vector3 by a quaternion
Vec3 Vec3::transform(const Vec3 &v, const Quat &q) {
  // v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
  Vec3 qv(q.x, q.y, q.z);
  Vec3 retVal = v;
  retVal += 2.0f * Vec3::cross(qv, Vec3::cross(qv, v) + q.w * v);
  return retVal;
}
} // namespace sinen