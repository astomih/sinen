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
class matrix3 {
public:
  float mat[3][3]{};
  /**
   * @brief Construct a new matrix3 object
   *
   */
  constexpr matrix3() { *this = matrix3::identity; }
  /**
   * @brief Construct a new matrix3 object
   *
   * @param inMat 3x3 array
   */
  explicit matrix3(float inMat[3][3]) { memcpy(mat, inMat, 9 * sizeof(float)); }
  /**
   * @brief Construct a new matrix3 object
   *
   * @param inMat 3x3 array
   */
  explicit matrix3(float inMat[9]) { memcpy(mat, inMat, 9 * sizeof(float)); }

  // Cast to a const float pointer
  [[nodiscard]] const float *get_ptr() const {
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
  static matrix3 create_scale(float xScale, float yScale) {
    float temp[3][3] = {
        {xScale, 0.0f, 0.0f},
        {0.0f, yScale, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return matrix3(temp);
  }

  static matrix3 create_scale(const vector2 &scaleVector) {
    return create_scale(scaleVector.x, scaleVector.y);
  }

  // Create a scale matrix with a uniform factor
  static matrix3 create_scale(float scale) {
    return create_scale(scale, scale);
  }

  // Create a rotation matrix about the Z axis
  // theta is in radians
  static matrix3 create_rotation(float theta) {
    float temp[3][3] = {
        {math::cos(theta), math::sin(theta), 0.0f},
        {-math::sin(theta), math::cos(theta), 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    return matrix3(temp);
  }

  // Create a translation matrix (on the xy-plane)
  static matrix3 create_translation(const vector2 &trans) {
    float temp[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {trans.x, trans.y, 1.0f},
    };
    return matrix3(temp);
  }

  static const matrix3 identity;
};
} // namespace sinen
#endif // !SINEN_MATRIX3_HPP