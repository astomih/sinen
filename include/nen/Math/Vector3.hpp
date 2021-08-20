#pragma once
#include "Math.hpp"
namespace nen
{
    // 3D Vector
    class Vector3
    {
    public:
        float x;
        float y;
        float z;

        constexpr Vector3()
            : x(0.f), y(0.f), z(0.f)
        {
        }
        constexpr explicit Vector3(const float value)
            : x(value), y(value), z(value)
        {
        }

        constexpr explicit Vector3(const float x, const float y, const float z)
            : x(x), y(y), z(z)
        {
        }

        // Cast to a const float pointer
        [[nodiscard]] const float *GetAsFloatPtr() const
        {
            return reinterpret_cast<const float *>(&x);
        }

        // Set all three components in one line
        void Set(float inX, float inY, float inZ)
        {
            x = inX;
            y = inY;
            z = inZ;
        }

        // Vector addition (a + b)
        friend Vector3 operator+(const Vector3 &a, const Vector3 &b)
        {
            return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        // Vector subtraction (a - b)
        friend Vector3 operator-(const Vector3 &a, const Vector3 &b)
        {
            return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        // Component-wise multiplication
        friend Vector3 operator*(const Vector3 &left, const Vector3 &right)
        {
            return Vector3(left.x * right.x, left.y * right.y, left.z * right.z);
        }

        // Scalar multiplication
        friend Vector3 operator*(const Vector3 &vec, float scalar)
        {
            return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
        }

        // Scalar multiplication
        friend Vector3 operator*(float scalar, const Vector3 &vec)
        {
            return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
        }

        // Scalar *=
        Vector3 &operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        // Vector +=
        Vector3 &operator+=(const Vector3 &right)
        {
            x += right.x;
            y += right.y;
            z += right.z;
            return *this;
        }

        // Vector -=
        Vector3 &operator-=(const Vector3 &right)
        {
            x -= right.x;
            y -= right.y;
            z -= right.z;
            return *this;
        }

        // Length squared of vector
        [[nodiscard]] float LengthSq() const
        {
            return (x * x + y * y + z * z);
        }

        // Length of vector
        [[nodiscard]] float Length() const
        {
            return (Math::Sqrt(LengthSq()));
        }

        // Normalize this vector
        void Normalize()
        {
            const auto length = Length();
            x /= length;
            y /= length;
            z /= length;
        }

        // Normalize the provided vector
        static Vector3 Normalize(const Vector3 &vec)
        {
            auto temp = vec;
            temp.Normalize();
            return temp;
        }

        // Dot product between two vectors (a dot b)
        static float Dot(const Vector3 &a, const Vector3 &b)
        {
            return (a.x * b.x + a.y * b.y + a.z * b.z);
        }

        // Cross product between two vectors (a cross b)
        static Vector3 Cross(const Vector3 &a, const Vector3 &b)
        {
            Vector3 temp;
            temp.x = a.y * b.z - a.z * b.y;
            temp.y = a.z * b.x - a.x * b.z;
            temp.z = a.x * b.y - a.y * b.x;
            return temp;
        }

        // Lerp from A to B by f
        static Vector3 Lerp(const Vector3 &a, const Vector3 &b, float f)
        {
            return Vector3(a + f * (b - a));
        }

        // Reflect V about (normalized) N
        static Vector3 Reflect(const Vector3 &v, const Vector3 &n)
        {
            return v - 2.0f * Vector3::Dot(v, n) * n;
        }

        static Vector3 Transform(const Vector3 &vec, const class Matrix4 &mat, float w = 1.0f);
        // This will transform the vector and renormalize the w component
        static Vector3 TransformWithPerspDiv(const Vector3 &vec, const class Matrix4 &mat, float w = 1.0f);

        // Transform a Vector3f by a quaternion
        static Vector3 Transform(const Vector3 &v, const class Quaternion &q);

        /// <summary>
        /// VDE Vector3f to Effekseer Vector3D
        /// </summary>
        //Effekseer::Vector3D ToEffekseer() const;

        static const Vector3 Zero;
        static const Vector3 UnitX;
        static const Vector3 UnitY;
        static const Vector3 UnitZ;
        static const Vector3 NegUnitX;
        static const Vector3 NegUnitY;
        static const Vector3 NegUnitZ;
        static const Vector3 Infinity;
        static const Vector3 NegInfinity;
    };
}