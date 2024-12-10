#ifndef SINEN_MATRIX3_HPP
#define SINEN_MATRIX3_HPP
#include "math.hpp"
#include "vector2.hpp"
#include <string>
namespace sinen {
/**
 * @brief float matrix3x3 class
 *
 */
class Matrix3 {
public:
  float mat[3][3]{};
  /**
   * @brief Construct a new matrix3 object
   *
   */
  constexpr Matrix3() { *this = Matrix3::identity; }
  /**
   * @brief Construct a new matrix3 object
   *
   * @param inMat 3x3 array
   */
  explicit Matrix3(float inMat[3][3]) { memcpy(mat, inMat, 9 * sizeof(float)); }
  /**
   * @brief Construct a new matrix3 object
   *
   * @param inMat 3x3 array
   */
  explicit Matrix3(float inMat[9]) { memcpy(mat, inMat, 9 * sizeof(float)); }

  // Cast to a const float pointer
  [[nodiscard]] const float *get_ptr() const {
    return reinterpret_cast<const float *>(&mat[0][0]);
  }

  // Matrix multiplication
  friend Matrix3 operator*(const Matrix3 &left, const Matrix3 &right) {
    Matrix3 retVal;
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

  Matrix3 &operator*=(const Matrix3 &right) {
    *this = *this * right;
    return *this;
  }

  // Create a scale matrix with x and y scales
  static Matrix3 create_scale(float xScale, float yScale) {
    float temp[3][3] = {
        {xScale, 0.0f, 0.0f},
        {0.0f, yScale, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return Matrix3(temp);
  }

  static Matrix3 create_scale(const Vector2 &scaleVector) {
    return create_scale(scaleVector.x, scaleVector.y);
  }

  // Create a scale matrix with a uniform factor
  static Matrix3 create_scale(float scale) {
    return create_scale(scale, scale);
  }

  // Create a rotation matrix about the Z axis
  // theta is in radians
  static Matrix3 create_rotation(float theta) {
    float temp[3][3] = {
        {Math::cos(theta), Math::sin(theta), 0.0f},
        {-Math::sin(theta), Math::cos(theta), 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return Matrix3(temp);
  }

  // Create a translation matrix (on the xy-plane)
  static Matrix3 create_translation(const Vector2 &trans) {
    float temp[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {trans.x, trans.y, 1.0f},
    };
    return Matrix3(temp);
  }

  static const Matrix3 identity;
};
} // namespace sinen
#endif // !SINEN_MATRIX3_HPP