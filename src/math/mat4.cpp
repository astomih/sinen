#include "matrix.hpp"
#include "quaternion.hpp"
#include "vector.hpp"

namespace sinen {
constexpr Mat4::Mat4(float a00, float a01, float a02, float a03, float a10,
                     float a11, float a12, float a13, float a20, float a21,
                     float a22, float a23, float a30, float a31, float a32,
                     float a33)
    : m16(a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23, a30, a31,
          a32, a33) {}

Vec3 Mat4::operator*(const Vec3 &vec) const {

  float x =
      mat[0][0] * vec.x + mat[1][0] * vec.y + mat[2][0] * vec.z + mat[3][0];
  float y =
      mat[0][1] * vec.x + mat[1][1] * vec.y + mat[2][1] * vec.z + mat[3][1];
  float z =
      mat[0][2] * vec.x + mat[1][2] * vec.y + mat[2][2] * vec.z + mat[3][2];
  float w =
      mat[0][3] * vec.x + mat[1][3] * vec.y + mat[2][3] * vec.z + mat[3][3];

  return Vec3(x / w, y / w, z / w);
}
Vec4 Mat4::operator*(const Vec4 &vec) const {

  float x = mat[0][0] * vec.x + mat[1][0] * vec.y + mat[2][0] * vec.z +
            mat[3][0] * vec.w;
  float y = mat[0][1] * vec.x + mat[1][1] * vec.y + mat[2][1] * vec.z +
            mat[3][1] * vec.w;
  float z = mat[0][2] * vec.x + mat[1][2] * vec.y + mat[2][2] * vec.z +
            mat[3][2] * vec.w;
  float w = mat[0][3] * vec.x + mat[1][3] * vec.y + mat[2][3] * vec.z +
            mat[3][3] * vec.w;

  return Vec4(x, y, z, w);
}
Mat4 operator*(const Mat4 &a, const Mat4 &b) {
  Mat4 retVal;
  // row 0
  retVal.mat[0][0] = a.mat[0][0] * b.mat[0][0] + a.mat[0][1] * b.mat[1][0] +
                     a.mat[0][2] * b.mat[2][0] + a.mat[0][3] * b.mat[3][0];

  retVal.mat[0][1] = a.mat[0][0] * b.mat[0][1] + a.mat[0][1] * b.mat[1][1] +
                     a.mat[0][2] * b.mat[2][1] + a.mat[0][3] * b.mat[3][1];

  retVal.mat[0][2] = a.mat[0][0] * b.mat[0][2] + a.mat[0][1] * b.mat[1][2] +
                     a.mat[0][2] * b.mat[2][2] + a.mat[0][3] * b.mat[3][2];

  retVal.mat[0][3] = a.mat[0][0] * b.mat[0][3] + a.mat[0][1] * b.mat[1][3] +
                     a.mat[0][2] * b.mat[2][3] + a.mat[0][3] * b.mat[3][3];

  // row 1
  retVal.mat[1][0] = a.mat[1][0] * b.mat[0][0] + a.mat[1][1] * b.mat[1][0] +
                     a.mat[1][2] * b.mat[2][0] + a.mat[1][3] * b.mat[3][0];

  retVal.mat[1][1] = a.mat[1][0] * b.mat[0][1] + a.mat[1][1] * b.mat[1][1] +
                     a.mat[1][2] * b.mat[2][1] + a.mat[1][3] * b.mat[3][1];

  retVal.mat[1][2] = a.mat[1][0] * b.mat[0][2] + a.mat[1][1] * b.mat[1][2] +
                     a.mat[1][2] * b.mat[2][2] + a.mat[1][3] * b.mat[3][2];

  retVal.mat[1][3] = a.mat[1][0] * b.mat[0][3] + a.mat[1][1] * b.mat[1][3] +
                     a.mat[1][2] * b.mat[2][3] + a.mat[1][3] * b.mat[3][3];

  // row 2
  retVal.mat[2][0] = a.mat[2][0] * b.mat[0][0] + a.mat[2][1] * b.mat[1][0] +
                     a.mat[2][2] * b.mat[2][0] + a.mat[2][3] * b.mat[3][0];

  retVal.mat[2][1] = a.mat[2][0] * b.mat[0][1] + a.mat[2][1] * b.mat[1][1] +
                     a.mat[2][2] * b.mat[2][1] + a.mat[2][3] * b.mat[3][1];

  retVal.mat[2][2] = a.mat[2][0] * b.mat[0][2] + a.mat[2][1] * b.mat[1][2] +
                     a.mat[2][2] * b.mat[2][2] + a.mat[2][3] * b.mat[3][2];

  retVal.mat[2][3] = a.mat[2][0] * b.mat[0][3] + a.mat[2][1] * b.mat[1][3] +
                     a.mat[2][2] * b.mat[2][3] + a.mat[2][3] * b.mat[3][3];

  // row 3
  retVal.mat[3][0] = a.mat[3][0] * b.mat[0][0] + a.mat[3][1] * b.mat[1][0] +
                     a.mat[3][2] * b.mat[2][0] + a.mat[3][3] * b.mat[3][0];

  retVal.mat[3][1] = a.mat[3][0] * b.mat[0][1] + a.mat[3][1] * b.mat[1][1] +
                     a.mat[3][2] * b.mat[2][1] + a.mat[3][3] * b.mat[3][1];

  retVal.mat[3][2] = a.mat[3][0] * b.mat[0][2] + a.mat[3][1] * b.mat[1][2] +
                     a.mat[3][2] * b.mat[2][2] + a.mat[3][3] * b.mat[3][2];

  retVal.mat[3][3] = a.mat[3][0] * b.mat[0][3] + a.mat[3][1] * b.mat[1][3] +
                     a.mat[3][2] * b.mat[2][3] + a.mat[3][3] * b.mat[3][3];

  return retVal;
}

Mat4 Mat4::invert(const Mat4 &m) {
  // TODO: Better algorithm
  float tmp[12];
  float src[16];
  float dst[16];
  float det;

  float mat[4][4];
  memcpy(mat, m.mat, sizeof(float) * 16);
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
  return Mat4(mat);
}
void Mat4::inverse() { *this = invert(*this); }
Quat Mat4::toQuat(const Mat4 &m) {
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
    auto x = Math::sqrt(px) * 0.5f;
    auto d = 1 / (4 * x);
    return Quat(x, (m.mat[1][0] + m.mat[0][1]) * d,
                (m.mat[0][2] + m.mat[2][0]) * d,
                (m.mat[2][1] - m.mat[1][2]) * d);
  }
  case 1: {
    auto y = Math::sqrt(py) * 0.5f;
    auto d = 1 / (4 * y);
    return Quat((m.mat[1][0] + m.mat[0][1]) * d, y,
                (m.mat[2][1] + m.mat[1][2]) * d,
                (m.mat[0][2] - m.mat[2][0]) * d);
  }
  case 2: {
    auto z = Math::sqrt(pz) * 0.5f;
    auto d = 1 / (4 * z);
    return Quat((m.mat[0][2] + m.mat[2][0]) * d,
                (m.mat[2][1] + m.mat[1][2]) * d, z,
                (m.mat[1][0] - m.mat[0][1]) * d);
  }
  case 3: {
    auto w = Math::sqrt(pw) * 0.5f;
    auto d = 1 / (4 * w);
    return Quat((m.mat[2][1] - m.mat[1][2]) * d,
                (m.mat[0][2] - m.mat[2][0]) * d,
                (m.mat[1][0] - m.mat[0][1]) * d, w);
  }
  default:
    return Quat();
  }
}

Mat4 Mat4::lookat(const Vec3 &eye, const Vec3 &at, const Vec3 &up) {
  const auto FRONT = Vec3::normalize(eye - at);
  const auto RIGHT = Vec3::normalize(Vec3::cross(up, FRONT));
  const auto UP = Vec3::normalize(Vec3::cross(FRONT, RIGHT));

  const float tx = -Vec3::dot(RIGHT, eye);
  const float ty = -Vec3::dot(UP, eye);
  const float tz = -Vec3::dot(FRONT, eye);

  float temp[4][4] = {{RIGHT.x, RIGHT.y, RIGHT.z, tx},
                      {UP.x, UP.y, UP.z, ty},
                      {FRONT.x, FRONT.y, FRONT.z, tz},
                      {0.0f, 0.0f, 0.0f, 1.0f}};
  return Mat4(temp);
}

Mat4 Mat4::perspective(const float angle, const float aspect, const float near,
                       const float far) {
  const float yScale = Math::cot(angle / 2.0f);
  const float xScale = yScale / aspect;

  const float A = far / (near - far);
  const float B = (near * far) / (near - far);

  float temp[4][4] = {{xScale, 0.0f, 0.0f, 0.0f},
                      {0.0f, yScale, 0.0f, 0.0f},
                      {0.0f, 0.0f, A, B},
                      {0.0f, 0.0f, -1.0f, 0.0f}};
  return Mat4(temp);
}
Mat4 Mat4::ortho(float width, float height, float near, float far) {
  float temp[4][4] = {{2.0f / width, 0.0f, 0.0f, 0.0f},
                      {0.0f, 2.0f / height, 0.0f, 0.0f},
                      {0.0f, 0.0f, -1.0f / (far - near), near / (near - far)},
                      {0.0f, 0.0f, 0.0f, 1.0f}};
  return Mat4(temp);
}
Mat4 Mat4::transpose(const Mat4 &m) {
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
  return Mat4(mat);
}

Mat4 Mat4::translate(const Vec3 &trans) {
  float temp[4][4] = {{1.0f, 0.0f, 0.0f, trans.x},
                      {0.0f, 1.0f, 0.0f, trans.y},
                      {0.0f, 0.0f, 1.0f, trans.z},
                      {0.0f, 0.0f, 0.0f, 1.0f}};
  return Mat4(temp);
}
Mat4 Mat4::fromQuat(const Quat &q) {
  float mat[4][4];

  mat[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
  mat[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
  mat[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
  mat[0][3] = 0.0f;

  mat[1][0] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
  mat[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
  mat[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
  mat[1][3] = 0.0f;

  mat[2][0] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
  mat[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
  mat[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
  mat[2][3] = 0.0f;

  mat[3][0] = 0.0f;
  mat[3][1] = 0.0f;
  mat[3][2] = 0.0f;
  mat[3][3] = 1.0f;

  return Mat4(mat);
}

Mat4 Mat4::scale(const Vec3 &scale) {
  float temp[4][4] = {{scale.x, 0.0f, 0.0f, 0.0f},
                      {0.0f, scale.y, 0.0f, 0.0f},
                      {0.0f, 0.0f, scale.z, 0.0f},
                      {0.0f, 0.0f, 0.0f, 1.0f}};
  return Mat4(temp);
}
} // namespace sinen