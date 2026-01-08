#ifndef SINEN_MAT4_HPP
#define SINEN_MAT4_HPP
#include "quaternion.hpp"
namespace sinen {
class Mat4 {
public:
  union {
    float mat[4][4];
    float m16[16];
  };

  /**
   * @brief Construct a new matrix4 object
   *
   */
  constexpr Mat4() : mat() { *this = Mat4::identity(); }
  explicit Mat4(float v) { *this = scale(Vec3{v}); }
  /**
   * @brief Construct a new matrix4 object
   *
   * @param inMat  matrix4x4 to copy
   */
  explicit Mat4(float inMat[4][4]) { memcpy(mat, inMat, 16 * sizeof(float)); }
  /**
   * @brief Construct a new matrix4 object
   *
   * @param inMat  matrix4x4 to copy
   */
  explicit Mat4(float inMat[16]) { memcpy(m16, inMat, 16 * sizeof(float)); }
  explicit Mat4(const float *value) { memcpy(m16, value, sizeof(float) * 16); }
  constexpr Mat4(float a00, float a01, float a02, float a03, float a10,
                 float a11, float a12, float a13, float a20, float a21,
                 float a22, float a23, float a30, float a31, float a32,
                 float a33);

  static constexpr Mat4 identity() {
    return Mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  }

  /**
   * @brief Cast to const float*
   *
   * @return const float* pointer to matrix4x4
   */
  float *get() { return reinterpret_cast<float *>(m16); }

  float *operator[](const size_t index) { return mat[index]; }
  const float *operator[](const size_t index) const { return mat[index]; }

  Vec3 operator*(const Vec3 &vec) const;

  Vec4 operator*(const Vec4 &vec) const;

  // Matrix multiplication (a * b)
  friend Mat4 operator*(const Mat4 &a, const Mat4 &b);

  Mat4 &operator*=(const Mat4 &right) {
    *this = *this * right;
    return *this;
  }

  // Invert the matrix - super slow
  void inverse();
  static Mat4 invert(const Mat4 &v);

  // Get the translation component of the matrix
  [[nodiscard]] Vec3 getTransform() const {
    return Vec3(mat[0][3], mat[1][3], mat[2][3]);
  }

  // Get the X axis of the matrix (forward)
  [[nodiscard]] Vec3 getXAxis() const {
    return Vec3::normalize(Vec3(mat[0][0], mat[0][1], mat[0][2]));
  }

  // Get the Y axis of the matrix (left)
  [[nodiscard]] Vec3 getYAxis() const {
    return Vec3::normalize(Vec3(mat[1][0], mat[1][1], mat[1][2]));
  }

  // Get the Z axis of the matrix (up)
  [[nodiscard]] Vec3 getZAxis() const {
    return Vec3::normalize(Vec3(mat[2][0], mat[2][1], mat[2][2]));
  }

  // Extract the scale component from the matrix
  [[nodiscard]] Vec3 getScale() const {
    Vec3 retVal;
    retVal.x = Vec3(mat[0][0], mat[0][1], mat[0][2]).length();
    retVal.y = Vec3(mat[1][0], mat[1][1], mat[1][2]).length();
    retVal.z = Vec3(mat[2][0], mat[2][1], mat[2][2]).length();
    return retVal;
  }

  // Create a scale matrix with x, y, and z scales

  // Rotation about x-axis
  static Mat4 rotateX(const float theta) {
    float temp[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, Math::cos(theta), Math::sin(theta), 0.0f},
        {0.0f, -Math::sin(theta), Math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Mat4(temp);
  }

  // Rotation about y-axis
  static Mat4 rotateY(const float theta) {
    float temp[4][4] = {
        {Math::cos(theta), 0.0f, -Math::sin(theta), 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {Math::sin(theta), 0.0f, Math::cos(theta), 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Mat4(temp);
  }

  // Rotation about z-axis
  static Mat4 rotateZ(const float theta) {
    float temp[4][4] = {
        {Math::cos(theta), Math::sin(theta), 0.0f, 0.0f},
        {-Math::sin(theta), Math::cos(theta), 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Mat4(temp);
  }
  static Mat4 transpose(const Mat4 &m);

  // Create a rotation matrix from a Quaternion
  static Mat4 fromQuat(const class Quat &q);
  static Quat toQuat(const Mat4 &m);
  static Mat4 translate(const Vec3 &trans);
  static Mat4 scale(const Vec3 &scale);

  static Mat4 lookat(const Vec3 &eye, const Vec3 &at, const Vec3 &up);
  static Mat4 perspective(const float angle, const float aspect,
                          const float near, const float far);
  static Mat4 ortho(float width, float height, float near, float far);
};
} // namespace sinen
#endif