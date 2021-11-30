#pragma once
#include "Math.hpp"
#include "Vector3.hpp"
#include "Quaternion.hpp"
#include <string>
namespace nen
{
    // 4x4 Matrix
    class Matrix4
    {
    public:
        float mat[4][4]{};

        constexpr Matrix4()
        {
            *this = Matrix4::Identity;
        }

        explicit Matrix4(float inMat[4][4])
        {
            memcpy(mat, inMat, 16 * sizeof(float));
        }

        // Cast to a const float pointer
        [[nodiscard]] const float *GetAsFloatPtr() const
        {
            return reinterpret_cast<const float *>(&mat[0][0]);
        }

        float *operator[](const size_t index)
        {
            return mat[index];
        }
        // Matrix multiplication (a * b)
        friend Matrix4 operator*(const Matrix4 &a, const Matrix4 &b)
        {
            Matrix4 retVal;
            // row 0
            retVal.mat[0][0] =
                a.mat[0][0] * b.mat[0][0] +
                a.mat[0][1] * b.mat[1][0] +
                a.mat[0][2] * b.mat[2][0] +
                a.mat[0][3] * b.mat[3][0];

            retVal.mat[0][1] =
                a.mat[0][0] * b.mat[0][1] +
                a.mat[0][1] * b.mat[1][1] +
                a.mat[0][2] * b.mat[2][1] +
                a.mat[0][3] * b.mat[3][1];

            retVal.mat[0][2] =
                a.mat[0][0] * b.mat[0][2] +
                a.mat[0][1] * b.mat[1][2] +
                a.mat[0][2] * b.mat[2][2] +
                a.mat[0][3] * b.mat[3][2];

            retVal.mat[0][3] =
                a.mat[0][0] * b.mat[0][3] +
                a.mat[0][1] * b.mat[1][3] +
                a.mat[0][2] * b.mat[2][3] +
                a.mat[0][3] * b.mat[3][3];

            // row 1
            retVal.mat[1][0] =
                a.mat[1][0] * b.mat[0][0] +
                a.mat[1][1] * b.mat[1][0] +
                a.mat[1][2] * b.mat[2][0] +
                a.mat[1][3] * b.mat[3][0];

            retVal.mat[1][1] =
                a.mat[1][0] * b.mat[0][1] +
                a.mat[1][1] * b.mat[1][1] +
                a.mat[1][2] * b.mat[2][1] +
                a.mat[1][3] * b.mat[3][1];

            retVal.mat[1][2] =
                a.mat[1][0] * b.mat[0][2] +
                a.mat[1][1] * b.mat[1][2] +
                a.mat[1][2] * b.mat[2][2] +
                a.mat[1][3] * b.mat[3][2];

            retVal.mat[1][3] =
                a.mat[1][0] * b.mat[0][3] +
                a.mat[1][1] * b.mat[1][3] +
                a.mat[1][2] * b.mat[2][3] +
                a.mat[1][3] * b.mat[3][3];

            // row 2
            retVal.mat[2][0] =
                a.mat[2][0] * b.mat[0][0] +
                a.mat[2][1] * b.mat[1][0] +
                a.mat[2][2] * b.mat[2][0] +
                a.mat[2][3] * b.mat[3][0];

            retVal.mat[2][1] =
                a.mat[2][0] * b.mat[0][1] +
                a.mat[2][1] * b.mat[1][1] +
                a.mat[2][2] * b.mat[2][1] +
                a.mat[2][3] * b.mat[3][1];

            retVal.mat[2][2] =
                a.mat[2][0] * b.mat[0][2] +
                a.mat[2][1] * b.mat[1][2] +
                a.mat[2][2] * b.mat[2][2] +
                a.mat[2][3] * b.mat[3][2];

            retVal.mat[2][3] =
                a.mat[2][0] * b.mat[0][3] +
                a.mat[2][1] * b.mat[1][3] +
                a.mat[2][2] * b.mat[2][3] +
                a.mat[2][3] * b.mat[3][3];

            // row 3
            retVal.mat[3][0] =
                a.mat[3][0] * b.mat[0][0] +
                a.mat[3][1] * b.mat[1][0] +
                a.mat[3][2] * b.mat[2][0] +
                a.mat[3][3] * b.mat[3][0];

            retVal.mat[3][1] =
                a.mat[3][0] * b.mat[0][1] +
                a.mat[3][1] * b.mat[1][1] +
                a.mat[3][2] * b.mat[2][1] +
                a.mat[3][3] * b.mat[3][1];

            retVal.mat[3][2] =
                a.mat[3][0] * b.mat[0][2] +
                a.mat[3][1] * b.mat[1][2] +
                a.mat[3][2] * b.mat[2][2] +
                a.mat[3][3] * b.mat[3][2];

            retVal.mat[3][3] =
                a.mat[3][0] * b.mat[0][3] +
                a.mat[3][1] * b.mat[1][3] +
                a.mat[3][2] * b.mat[2][3] +
                a.mat[3][3] * b.mat[3][3];

            return retVal;
        }

        Matrix4 &operator*=(const Matrix4 &right)
        {
            *this = *this * right;
            return *this;
        }

        // Invert the matrix - super slow
        void Invert();

        // Get the translation component of the matrix
        [[nodiscard]] Vector3 GetTranslation() const
        {
            return Vector3(mat[3][0], mat[3][1], mat[3][2]);
        }

        // Get the X axis of the matrix (forward)
        [[nodiscard]] Vector3 GetXAxis() const
        {
            return Vector3::Normalize(Vector3(mat[0][0], mat[0][1], mat[0][2]));
        }

        // Get the Y axis of the matrix (left)
        [[nodiscard]] Vector3 GetYAxis() const
        {
            return Vector3::Normalize(Vector3(mat[1][0], mat[1][1], mat[1][2]));
        }

        // Get the Z axis of the matrix (up)
        [[nodiscard]] Vector3 GetZAxis() const
        {
            return Vector3::Normalize(Vector3(mat[2][0], mat[2][1], mat[2][2]));
        }

        // Extract the scale component from the matrix
        [[nodiscard]] Vector3 GetScale() const
        {
            Vector3 retVal;
            retVal.x = Vector3(mat[0][0], mat[0][1], mat[0][2]).Length();
            retVal.y = Vector3(mat[1][0], mat[1][1], mat[1][2]).Length();
            retVal.z = Vector3(mat[2][0], mat[2][1], mat[2][2]).Length();
            return retVal;
        }

        // Create a scale matrix with x, y, and z scales

        // Rotation about x-axis
        static Matrix4 CreateRotationX(const float theta)
        {
            float temp[4][4] =
                {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, Math::Cos(theta), Math::Sin(theta), 0.0f},
                    {0.0f, -Math::Sin(theta), Math::Cos(theta), 0.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f},
                };
            return Matrix4(temp);
        }

        // Rotation about y-axis
        static Matrix4 CreateRotationY(const float theta)
        {
            float temp[4][4] =
                {
                    {Math::Cos(theta), 0.0f, -Math::Sin(theta), 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {Math::Sin(theta), 0.0f, Math::Cos(theta), 0.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f},
                };
            return Matrix4(temp);
        }

        // Rotation about z-axis
        static Matrix4 CreateRotationZ(const float theta)
        {
            float temp[4][4] =
                {
                    {Math::Cos(theta), Math::Sin(theta), 0.0f, 0.0f},
                    {-Math::Sin(theta), Math::Cos(theta), 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f},
                };
            return Matrix4(temp);
        }

        // Create a rotation matrix from a quaternion
        static Matrix4 CreateFromQuaternion(const class Quaternion &q);

        static Matrix4 CreateTranslation(const Vector3 &trans);

        static Quaternion ToQuaternion(const Matrix4 &m);

        static Matrix4 LookAt(const Vector3 &eye, const Vector3 &at, const Vector3 &up);

        static Matrix4 Perspective(const float angle, const float aspect, const float near, const float far);

        static const Matrix4 Identity;
    };
}