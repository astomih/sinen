#pragma once
#include "math.hpp"
#include "quaternion.hpp"
#include "vector3.hpp"
#include <string>

namespace nen {
// 4x4 Matrix
class matrix4 {
public:
  float mat[4][4]{};

  constexpr matrix4() { *this = matrix4::Identity; }

  explicit matrix4(float inMat[4][4]) {
    memcpy(mat, inMat, 16 * sizeof(float));
  }

  // Cast to a const float pointer
  [[nodiscard]] const float *GetAsFloatPtr() const {
    return reinterpret_cast<const float *>(&mat[0][0]);
  }

  float *operator[](const size_t index) { return mat[index]; }

  vector3 operator*(const vector3 &vec) const;

  // Matrix multiplication (a * b)
  friend matrix4 operator*(const matrix4 &a, const matrix4 &b) {
    matrix4 retVal;
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

  matrix4 &operator*=(const matrix4 &right) {
    *this = *this * right;
    return *this;
  }

  // Invert the matrix - super slow
  void Invert();

  // Get the translation component of the matrix
  [[nodiscard]] vector3 GetTranslation() const {
    return vector3(mat[3][0], mat[3][1], mat[3][2]);
  }

  // Get the X axis of the matrix (forward)
  [[nodiscard]] vector3 GetXAxis() const {
    return vector3::Normalize(vector3(mat[0][0], mat[0][1], mat[0][2]));
  }

  // Get the Y axis of the matrix (left)
  [[nodiscard]] vector3 GetYAxis() const {
    return vector3::Normalize(vector3(mat[1][0], mat[1][1], mat[1][2]));
  }

  // Get the Z axis of the matrix (up)
  [[nodiscard]] vector3 GetZAxis() const {
    return vector3::Normalize(vector3(mat[2][0], mat[2][1], mat[2][2]));
  }

  // Extract the scale component from the matrix
  [[nodiscard]] vector3 GetScale() const {
    vector3 retVal;
    retVal.x = vector3(mat[0][0], mat[0][1], mat[0][2]).Length();
    retVal.y = vector3(mat[1][0], mat[1][1], mat[1][2]).Length();
    retVal.z = vector3(mat[2][0], mat[2][1], mat[2][2]).Length();
    return retVal;
  }

  // Create a scale matrix with x, y, and z scales

  // Rotation about x-axis
  static matrix4 CreateRotationX(const float theta) {
    float temp[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, math::Cos(theta), math::Sin(theta), 0.0f},
        {0.0f, -math::Sin(theta), math::Cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }

  // Rotation about y-axis
  static matrix4 CreateRotationY(const float theta) {
    float temp[4][4] = {
        {math::Cos(theta), 0.0f, -math::Sin(theta), 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {math::Sin(theta), 0.0f, math::Cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }

  // Rotation about z-axis
  static matrix4 CreateRotationZ(const float theta) {
    float temp[4][4] = {
        {math::Cos(theta), math::Sin(theta), 0.0f, 0.0f},
        {-math::Sin(theta), math::Cos(theta), 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }
  static matrix4 Transpose(const matrix4 &m);

  // Create a rotation matrix from a quaternion
  static matrix4 CreateFromQuaternion(const class quaternion &q);

  static matrix4 CreateTranslation(const vector3 &trans);

  static quaternion ToQuaternion(const matrix4 &m);

  static matrix4 LookAt(const vector3 &eye, const vector3 &at,
                        const vector3 &up);

  static matrix4 Perspective(const float angle, const float aspect,
                             const float near, const float far);

  static const matrix4 Identity;
};
} // namespace nen