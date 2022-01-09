#pragma once
#include "Math.hpp"
#include "Vector2.hpp"
#include <string>
namespace nen {
// 3x3 Matrix
class matrix3 {
public:
  float mat[3][3] = {};

  matrix3() { *this = matrix3::Identity; }

  explicit matrix3(float inMat[3][3]) { memcpy(mat, inMat, 9 * sizeof(float)); }

  // Cast to a const float pointer
  [[nodiscard]] const float *GetAsFloatPtr() const {
    return reinterpret_cast<const float *>(&mat[0][0]);
  }

  // Matrix multiplication
  friend matrix3 operator*(const matrix3 &left, const matrix3 &right) {
    matrix3 retVal;
    // row 0
    retVal.mat[0][0] = left.mat[0][0] * right.mat[0][0] +
                       left.mat[0][1] * right.mat[1][0] +
                       left.mat[0][2] * right.mat[2][0];

    retVal.mat[0][1] = left.mat[0][0] * right.mat[0][1] +
                       left.mat[0][1] * right.mat[1][1] +
                       left.mat[0][2] * right.mat[2][1];

    retVal.mat[0][2] = left.mat[0][0] * right.mat[0][2] +
                       left.mat[0][1] * right.mat[1][2] +
                       left.mat[0][2] * right.mat[2][2];

    // row 1
    retVal.mat[1][0] = left.mat[1][0] * right.mat[0][0] +
                       left.mat[1][1] * right.mat[1][0] +
                       left.mat[1][2] * right.mat[2][0];

    retVal.mat[1][1] = left.mat[1][0] * right.mat[0][1] +
                       left.mat[1][1] * right.mat[1][1] +
                       left.mat[1][2] * right.mat[2][1];

    retVal.mat[1][2] = left.mat[1][0] * right.mat[0][2] +
                       left.mat[1][1] * right.mat[1][2] +
                       left.mat[1][2] * right.mat[2][2];

    // row 2
    retVal.mat[2][0] = left.mat[2][0] * right.mat[0][0] +
                       left.mat[2][1] * right.mat[1][0] +
                       left.mat[2][2] * right.mat[2][0];

    retVal.mat[2][1] = left.mat[2][0] * right.mat[0][1] +
                       left.mat[2][1] * right.mat[1][1] +
                       left.mat[2][2] * right.mat[2][1];

    retVal.mat[2][2] = left.mat[2][0] * right.mat[0][2] +
                       left.mat[2][1] * right.mat[1][2] +
                       left.mat[2][2] * right.mat[2][2];

    return retVal;
  }

  matrix3 &operator*=(const matrix3 &right) {
    *this = *this * right;
    return *this;
  }

  // Create a scale matrix with x and y scales
  static matrix3 CreateScale(float xScale, float yScale) {
    float temp[3][3] = {
        {xScale, 0.0f, 0.0f},
        {0.0f, yScale, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return matrix3(temp);
  }

  static matrix3 CreateScale(const vector2 &scaleVector) {
    return CreateScale(scaleVector.x, scaleVector.y);
  }

  // Create a scale matrix with a uniform factor
  static matrix3 CreateScale(float scale) { return CreateScale(scale, scale); }

  // Create a rotation matrix about the Z axis
  // theta is in radians
  static matrix3 CreateRotation(float theta) {
    float temp[3][3] = {
        {Math::Cos(theta), Math::Sin(theta), 0.0f},
        {-Math::Sin(theta), Math::Cos(theta), 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return matrix3(temp);
  }

  // Create a translation matrix (on the xy-plane)
  static matrix3 CreateTranslation(const vector2 &trans) {
    float temp[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {trans.x, trans.y, 1.0f},
    };
    return matrix3(temp);
  }

  static const matrix3 Identity;
};
} // namespace nen