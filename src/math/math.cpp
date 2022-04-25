#include <math/math.hpp>
#include <math/matrix3.hpp>
#include <math/matrix4.hpp>
#include <math/vector2.hpp>
#include <math/vector3.hpp>
namespace nen {
const vector2 vector2::zero(0.0f, 0.0f);
const vector2 vector2::unit_x(1.0f, 0.0f);
const vector2 vector2::unit_y(0.0f, 1.0f);
const vector2 vector2::neg_unit_x(-1.0f, 0.0f);
const vector2 vector2::neg_unit_y(0.0f, -1.0f);

const vector3 vector3::zero(0.0f, 0.0f, 0.f);
const vector3 vector3::unit_x(1.0f, 0.0f, 0.0f);
const vector3 vector3::unit_y(0.0f, 1.0f, 0.0f);
const vector3 vector3::unit_z(0.0f, 0.0f, 1.0f);
const vector3 vector3::neg_unit_x(-1.0f, 0.0f, 0.0f);
const vector3 vector3::neg_unit_y(0.0f, -1.0f, 0.0f);
const vector3 vector3::neg_unit_z(0.0f, 0.0f, -1.0f);
const vector3 vector3::infinity(math::Infinity, math::Infinity, math::Infinity);
const vector3 vector3::neg_infinity(math::NegInfinity, math::NegInfinity,
                                    math::NegInfinity);

static float m3Ident[3][3] = {
    {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
const matrix3 matrix3::identity(m3Ident);

static float m4Ident[4][4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                              {0.0f, 1.0f, 0.0f, 0.0f},
                              {0.0f, 0.0f, 1.0f, 0.0f},
                              {0.0f, 0.0f, 0.0f, 1.0f}};

const matrix4 matrix4::Identity(m4Ident);

const quaternion quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

vector2 vector2::transform(const vector2 &vec, const matrix3 &mat,
                           float w /*= 1.0f*/) {
  vector2 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] + w * mat.mat[2][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] + w * mat.mat[2][1];
  // ignore w since we aren't returning a new value for it...
  return retVal;
}

vector3 vector3::transform(const vector3 &vec, const matrix4 &mat,
                           float w /*= 1.0f*/) {
  vector3 retVal;
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
vector3 vector3::transform_with_persp_div(const vector3 &vec,
                                          const matrix4 &mat,
                                          float w /*= 1.0f*/) {
  vector3 retVal;
  retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
             vec.z * mat.mat[2][0] + w * mat.mat[3][0];
  retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
             vec.z * mat.mat[2][1] + w * mat.mat[3][1];
  retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
             vec.z * mat.mat[2][2] + w * mat.mat[3][2];
  float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
                       vec.z * mat.mat[2][3] + w * mat.mat[3][3];
  if (!math::NearZero(math::Abs(transformedW))) {
    transformedW = 1.0f / transformedW;
    retVal *= transformedW;
  }
  return retVal;
}

// Transform a Vector3 by a quaternion
vector3 vector3::Transform(const vector3 &v, const quaternion &q) {
  // v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
  vector3 qv(q.x, q.y, q.z);
  vector3 retVal = v;
  retVal += 2.0f * vector3::Cross(qv, vector3::Cross(qv, v) + q.w * v);
  return retVal;
}
vector3 matrix4::operator*(const vector3 &vec) const {

  float x =
      mat[0][0] * vec.x + mat[1][0] * vec.y + mat[2][0] * vec.z + mat[3][0];
  float y =
      mat[0][1] * vec.x + mat[1][1] * vec.y + mat[2][1] * vec.z + mat[3][1];
  float z =
      mat[0][2] * vec.x + mat[1][2] * vec.y + mat[2][2] * vec.z + mat[3][2];
  float w =
      mat[0][3] * vec.x + mat[1][3] * vec.y + mat[2][3] * vec.z + mat[3][3];

  return vector3(x / w, y / w, z / w);
}

void matrix4::Invert() {
  // Thanks slow math
  // This is a really janky way to unroll everything...
  float tmp[12];
  float src[16];
  float dst[16];
  float det;

  // Transpose matrix
  // row 1 to col 1
  src[0] = mat[0][0];
  src[4] = mat[0][1];
  src[8] = mat[0][2];
  src[12] = mat[0][3];

  // row 2 to col 2
  src[1] = mat[1][0];
  src[5] = mat[1][1];
  src[9] = mat[1][2];
  src[13] = mat[1][3];

  // row 3 to col 3
  src[2] = mat[2][0];
  src[6] = mat[2][1];
  src[10] = mat[2][2];
  src[14] = mat[2][3];

  // row 4 to col 4
  src[3] = mat[3][0];
  src[7] = mat[3][1];
  src[11] = mat[3][2];
  src[15] = mat[3][3];

  // Calculate cofactors
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];

  dst[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
  dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
  dst[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
  dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
  dst[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
  dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
  dst[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
  dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
  dst[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
  dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
  dst[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
  dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
  dst[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
  dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
  dst[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
  dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

  tmp[0] = src[2] * src[7];
  tmp[1] = src[3] * src[6];
  tmp[2] = src[1] * src[7];
  tmp[3] = src[3] * src[5];
  tmp[4] = src[1] * src[6];
  tmp[5] = src[2] * src[5];
  tmp[6] = src[0] * src[7];
  tmp[7] = src[3] * src[4];
  tmp[8] = src[0] * src[6];
  tmp[9] = src[2] * src[4];
  tmp[10] = src[0] * src[5];
  tmp[11] = src[1] * src[4];

  dst[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
  dst[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
  dst[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
  dst[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
  dst[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
  dst[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
  dst[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
  dst[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
  dst[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
  dst[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
  dst[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
  dst[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
  dst[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
  dst[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
  dst[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
  dst[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

  // Calculate determinant
  det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];

  // Inverse of matrix is divided by determinant
  det = 1 / det;
  for (int j = 0; j < 16; j++) {
    dst[j] *= det;
  }

  // Set it back
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      mat[i][j] = dst[i * 4 + j];
    }
  }
}
quaternion matrix4::ToQuaternion(const matrix4 &m) {
  auto px = m.mat[0][0] - m.mat[1][1] - m.mat[2][2] + 1;
  auto py = -m.mat[0][0] + m.mat[1][1] - m.mat[2][2] + 1;
  auto pz = -m.mat[0][0] - m.mat[1][1] + m.mat[2][2] + 1;
  auto pw = m.mat[0][0] + m.mat[1][1] + m.mat[2][2] + 1;

  int flag = 0;
  float max = px;
  if (max < py) {
    flag = 1;
    max = py;
  }
  if (max < pz) {
    flag = 2;
    max = pz;
  }
  if (max < pw) {
    flag = 3;
    max = pw;
  }
  switch (flag) {
  case 0: {
    auto x = math::Sqrt(px) * 0.5f;
    auto d = 1 / (4 * x);
    return quaternion(x, (m.mat[1][0] + m.mat[0][1]) * d,
                      (m.mat[0][2] + m.mat[2][0]) * d,
                      (m.mat[2][1] - m.mat[1][2]) * d);
  }
  case 1: {
    auto y = math::Sqrt(py) * 0.5f;
    auto d = 1 / (4 * y);
    return quaternion((m.mat[1][0] + m.mat[0][1]) * d, y,
                      (m.mat[2][1] + m.mat[1][2]) * d,
                      (m.mat[0][2] - m.mat[2][0]) * d);
  }
  case 2: {
    auto z = math::Sqrt(pz) * 0.5f;
    auto d = 1 / (4 * z);
    return quaternion((m.mat[0][2] + m.mat[2][0]) * d,
                      (m.mat[2][1] + m.mat[1][2]) * d, z,
                      (m.mat[1][0] - m.mat[0][1]) * d);
  }
  case 3: {
    auto w = math::Sqrt(pw) * 0.5f;
    auto d = 1 / (4 * w);
    return quaternion((m.mat[2][1] - m.mat[1][2]) * d,
                      (m.mat[0][2] - m.mat[2][0]) * d,
                      (m.mat[1][0] - m.mat[0][1]) * d, w);
  }
  default:
    return quaternion();
  }
}

matrix4 matrix4::LookAt(const vector3 &eye, const vector3 &at,
                        const vector3 &up) {
  const auto FRONT = vector3::Normalize(eye - at);
  const auto RIGHT = vector3::Normalize(vector3::Cross(up, FRONT));
  const auto UP = vector3::Normalize(vector3::Cross(FRONT, RIGHT));
  vector3 trans;
  trans.x = -vector3::Dot(RIGHT, eye);
  trans.y = -vector3::Dot(UP, eye);
  trans.z = -vector3::Dot(FRONT, eye);

  float temp[4][4] = {{RIGHT.x, UP.x, FRONT.x, 0.0f},
                      {RIGHT.y, UP.y, FRONT.y, 0.0f},
                      {RIGHT.z, UP.z, FRONT.z, 0.0f},
                      {trans.x, trans.y, trans.z, 1.0f}};
  return matrix4(temp);
}

matrix4 matrix4::Perspective(const float angle, const float aspect,
                             const float near, const float far) {
  const auto yScale = math::Cot(angle / 2.0f);
  const auto xScale = yScale / aspect;
  float temp[4][4] = {{xScale, 0.0f, 0.0f, 0.0f},
                      {0.0f, yScale, 0.0f, 0.0f},
                      {0.0f, 0.0f, far / (near - far), -1.0f},
                      {0.0f, 0.0f, near * far / (near - far), 0.0f}};
  return matrix4(temp);
}

matrix4 matrix4::Transpose(const matrix4 &m) {
  float mat[4][4];

  mat[0][0] = m.mat[0][0];
  mat[0][1] = m.mat[1][0];
  mat[0][2] = m.mat[2][0];
  mat[0][3] = m.mat[3][0];
  mat[1][0] = m.mat[0][1];
  mat[1][1] = m.mat[1][1];
  mat[1][2] = m.mat[2][1];
  mat[1][3] = m.mat[3][1];
  mat[2][0] = m.mat[0][2];
  mat[2][1] = m.mat[1][2];
  mat[2][2] = m.mat[2][2];
  mat[2][3] = m.mat[3][2];
  mat[3][0] = m.mat[0][3];
  mat[3][1] = m.mat[1][3];
  mat[3][2] = m.mat[2][3];
  mat[3][3] = m.mat[3][3];
  return matrix4(mat);
}

matrix4 matrix4::CreateTranslation(const vector3 &trans) {
  float temp[4][4] = {{1.0f, 0.0f, 0.0f, 0.0f},
                      {0.0f, 1.0f, 0.0f, 0.0f},
                      {0.0f, 0.0f, 1.0f, 0.0f},
                      {trans.x, trans.y, trans.z, 1.0f}};
  return matrix4(temp);
}

matrix4 matrix4::CreateFromQuaternion(const class quaternion &q) {
  float mat[4][4];

  mat[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
  mat[0][1] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
  mat[0][2] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
  mat[0][3] = 0.0f;

  mat[1][0] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
  mat[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
  mat[1][2] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
  mat[1][3] = 0.0f;

  mat[2][0] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
  mat[2][1] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
  mat[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
  mat[2][3] = 0.0f;

  mat[3][0] = 0.0f;
  mat[3][1] = 0.0f;
  mat[3][2] = 0.0f;
  mat[3][3] = 1.0f;

  return matrix4(mat);
}

vector3 quaternion::ToEuler(const quaternion &r) {
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
    tx = math::Pi / 2.f;
    ty = 0;
    tz = math::Atan2(m10, m00);
  } else if (m21 >= -1.01f && m21 <= -0.99f) {
    tx = -math::Pi / 2.f;
    ty = 0;
    tz = math::Atan2(m10, m00);
  } else {
    tx = std::asin(-m21);
    ty = math::Atan2(m20, m22);
    tz = math::Atan2(m01, m11);
  }

  return vector3(tx, ty, tz);
}

} // namespace nen