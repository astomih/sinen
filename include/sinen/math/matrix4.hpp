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

  constexpr matrix4() { *this = matrix4::identity; }

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
  void invert();

  // Get the translation component of the matrix
  [[nodiscard]] vector3 get_translation() const {
    return vector3(mat[3][0], mat[3][1], mat[3][2]);
  }

  // Get the X axis of the matrix (forward)
  [[nodiscard]] vector3 get_x_axis() const {
    return vector3::normalize(vector3(mat[0][0], mat[0][1], mat[0][2]));
  }

  // Get the Y axis of the matrix (left)
  [[nodiscard]] vector3 get_y_axis() const {
    return vector3::normalize(vector3(mat[1][0], mat[1][1], mat[1][2]));
  }

  // Get the Z axis of the matrix (up)
  [[nodiscard]] vector3 get_z_axis() const {
    return vector3::normalize(vector3(mat[2][0], mat[2][1], mat[2][2]));
  }

  // Extract the scale component from the matrix
  [[nodiscard]] vector3 get_scale() const {
    vector3 retVal;
    retVal.x = vector3(mat[0][0], mat[0][1], mat[0][2]).length();
    retVal.y = vector3(mat[1][0], mat[1][1], mat[1][2]).length();
    retVal.z = vector3(mat[2][0], mat[2][1], mat[2][2]).length();
    return retVal;
  }

  // Create a scale matrix with x, y, and z scales

  // Rotation about x-axis
  static matrix4 create_rotaion_x(const float theta) {
    float temp[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, math::cos(theta), math::sin(theta), 0.0f},
        {0.0f, -math::sin(theta), math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }

  // Rotation about y-axis
  static matrix4 create_rotation_y(const float theta) {
    float temp[4][4] = {
        {math::cos(theta), 0.0f, -math::sin(theta), 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {math::sin(theta), 0.0f, math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }

  // Rotation about z-axis
  static matrix4 create_rotation_z(const float theta) {
    float temp[4][4] = {
        {math::cos(theta), math::sin(theta), 0.0f, 0.0f},
        {-math::sin(theta), math::cos(theta), 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return matrix4(temp);
  }
  static matrix4 transpose(const matrix4 &m);

  // Create a rotation matrix from a quaternion
  static matrix4 create_from_quaternion(const class quaternion &q);

  static matrix4 create_translation(const vector3 &trans);

  static quaternion to_quaternion(const matrix4 &m);

  static matrix4 lookat(const vector3 &eye, const vector3 &at,
                        const vector3 &up);

  static matrix4 perspective(const float angle, const float aspect,
                             const float near, const float far);

  static matrix4 ortho(float width, float height, float near, float far);

  static const matrix4 identity;
};
} // namespace nen