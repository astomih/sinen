#pragma once
#include "Math.hpp"

namespace nen
{
    class Vector2
    {
    public:
        float x;
        float y;

        constexpr Vector2()
            : x(0.0f), y(0.0f)
        {
        }

        constexpr explicit Vector2(float inX, float inY)
            : x(inX), y(inY)
        {
        }

        // Set both components in one line
        void Set(float inX, float inY)
        {
            x = inX;
            y = inY;
        }

        const float *GetAsFloatPtr() const
        {
            return reinterpret_cast<const float *>(&x);
        }
        // Vector addition (a + b)
        friend Vector2 operator+(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x + b.x, a.y + b.y);
        }

        // Vector subtraction (a - b)
        friend Vector2 operator-(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x - b.x, a.y - b.y);
        }

        // Component-wise multiplication
        // (a.x * b.x, ...)
        friend Vector2 operator*(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x * b.x, a.y * b.y);
        }

        // Scalar multiplication
        friend Vector2 operator*(const Vector2 &vec, float scalar)
        {
            return Vector2(vec.x * scalar, vec.y * scalar);
        }

        // Scalar multiplication
        friend Vector2 operator*(float scalar, const Vector2 &vec)
        {
            return Vector2(vec.x * scalar, vec.y * scalar);
        }

        Vector2 &operator/=(float scalar)
        {

            x > 0.f ? x /= scalar : x = 0.f;
            y > 0.f ? y /= scalar : y = 0.f;
            return *this;
        }

        // Scalar *=
        Vector2 &operator*=(float scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        // Vector +=
        Vector2 &operator+=(const Vector2 &right)
        {
            x += right.x;
            y += right.y;
            return *this;
        }

        // Vector -=
        Vector2 &operator-=(const Vector2 &right)
        {
            x -= right.x;
            y -= right.y;
            return *this;
        }

        // Length squared of vector
        [[nodiscard]] float LengthSq() const
        {
            return (x * x + y * y);
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
        }

        // Normalize the provided vector
        static Vector2 Normalize(const Vector2 &vec)
        {
            auto temp = vec;
            temp.Normalize();
            return temp;
        }

        // Dot product between two vectors (a dot b)
        static float Dot(const Vector2 &a, const Vector2 &b)
        {
            return (a.x * b.x + a.y * b.y);
        }

        // Lerp from A to B by f
        static Vector2 Lerp(const Vector2 &a, const Vector2 &b, float f)
        {
            return Vector2(a + f * (b - a));
        }

        // Reflect V about (normalized) N
        static Vector2 Reflect(const Vector2 &v, const Vector2 &n)
        {
            return v - 2.0f * Vector2::Dot(v, n) * n;
        }

        // Transform vector by matrix
        static Vector2 Transform(const Vector2 &vec, const class Matrix3 &mat, float w = 1.0f);

        static const Vector2 Zero;
        static const Vector2 UnitX;
        static const Vector2 UnitY;
        static const Vector2 NegUnitX;
        static const Vector2 NegUnitY;
    };
}