#pragma once
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>
#include <SDL.h>
#include <cstring>
#include <algorithm>

namespace nen::Math
{
	constexpr float Pi = 3.1415926535f;
	constexpr float TwoPi = Pi * 2.0f;
	constexpr float PiPiP = Pi * (5.f / 2.f);
	constexpr float PiOver2 = Pi / 2.0f;
	constexpr float Infinity = std::numeric_limits<float>::infinity();
	constexpr float NegInfinity = -std::numeric_limits<float>::infinity();

	/// <summary>
	/// Degrees to radians
	/// </summary>
	/// <param name="degrees">Degrees</param>
	/// <returns>Radians</returns>
	inline float ToRadians(float degrees)
	{
		return degrees * Pi / 180.0f;
	}

	/// <summary>
	/// Radians to Degrees
	/// </summary>
	/// <param name="radians">Radians</param>
	/// <returns>Degrees</returns>
	inline float ToDegrees(float radians)
	{
		return radians * 180.0f / Pi;
	}

	/// <summary>
	/// explore near zero?
	/// </summary>
	/// <param name="val"></param>
	/// <param name="epsilon"></param>
	/// <returns></returns>
	inline bool NearZero(float val, float epsilon = 0.001f)
	{
		if (fabs(val) <= epsilon)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	template <typename T>
	T Max(const T &a, const T &b)
	{
		return (a < b ? b : a);
	}

	template <typename T>
	T Min(const T &a, const T &b)
	{
		return (a < b ? a : b);
	}

	template <typename T>
	T Clamp(const T &value, const T &min, const T &max)
	{
		return std::clamp(value, min, max);
	}

	inline float Abs(float value)
	{
		return fabs(value);
	}

	inline float Cos(float angle)
	{
		return cosf(angle);
	}

	inline float Sin(float angle)
	{
		return sinf(angle);
	}

	inline float Tan(float angle)
	{
		return tanf(angle);
	}

	inline float Acos(float value)
	{
		return acosf(value);
	}

	inline float Atan2(float y, float x)
	{
		return atan2f(y, x);
	}

	inline float Cot(float angle)
	{
		return 1.0f / Tan(angle);
	}

	inline float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	inline float Sqrt(float value)
	{
		return sqrtf(value);
	}

	inline float Fmod(float numer, float denom)
	{
		return fmod(numer, denom);
	}

	float inline Sin0_1(const float periodSec, const float t = static_cast<float>(SDL_GetTicks()) / 1000.f)
	{
		const auto f = std::fmod(t, periodSec);
		const auto x = f / (periodSec * (1.f / (2.f * Pi)));
		return std::sin(x) * 0.5f + 0.5f;
	}

	float inline Cos0_1(const float periodSec, const float t = static_cast<float>(SDL_GetTicks()) / 1000.f)
	{
		const auto f = std::fmod(t, periodSec);
		const auto x = f / (periodSec * (1.f / (2.f * Pi)));
		return std::cos(x) * 0.5f + 0.5f;
	}
} // namespace Math

namespace nen
{
	// 2D Vector
	class Vector2f
	{
	public:
		float x;
		float y;

		constexpr Vector2f()
			: x(0.0f), y(0.0f)
		{
		}

		constexpr explicit Vector2f(float inX, float inY)
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
		friend Vector2f operator+(const Vector2f &a, const Vector2f &b)
		{
			return Vector2f(a.x + b.x, a.y + b.y);
		}

		// Vector subtraction (a - b)
		friend Vector2f operator-(const Vector2f &a, const Vector2f &b)
		{
			return Vector2f(a.x - b.x, a.y - b.y);
		}

		// Component-wise multiplication
		// (a.x * b.x, ...)
		friend Vector2f operator*(const Vector2f &a, const Vector2f &b)
		{
			return Vector2f(a.x * b.x, a.y * b.y);
		}

		// Scalar multiplication
		friend Vector2f operator*(const Vector2f &vec, float scalar)
		{
			return Vector2f(vec.x * scalar, vec.y * scalar);
		}

		// Scalar multiplication
		friend Vector2f operator*(float scalar, const Vector2f &vec)
		{
			return Vector2f(vec.x * scalar, vec.y * scalar);
		}

		Vector2f &operator/=(float scalar)
		{

			x > 0.f ? x /= scalar : x = 0.f;
			y > 0.f ? y /= scalar : y = 0.f;
			return *this;
		}

		// Scalar *=
		Vector2f &operator*=(float scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		// Vector +=
		Vector2f &operator+=(const Vector2f &right)
		{
			x += right.x;
			y += right.y;
			return *this;
		}

		// Vector -=
		Vector2f &operator-=(const Vector2f &right)
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
		static Vector2f Normalize(const Vector2f &vec)
		{
			auto temp = vec;
			temp.Normalize();
			return temp;
		}

		// Dot product between two vectors (a dot b)
		static float Dot(const Vector2f &a, const Vector2f &b)
		{
			return (a.x * b.x + a.y * b.y);
		}

		// Lerp from A to B by f
		static Vector2f Lerp(const Vector2f &a, const Vector2f &b, float f)
		{
			return Vector2f(a + f * (b - a));
		}

		// Reflect V about (normalized) N
		static Vector2f Reflect(const Vector2f &v, const Vector2f &n)
		{
			return v - 2.0f * Vector2f::Dot(v, n) * n;
		}

		// Transform vector by matrix
		static Vector2f Transform(const Vector2f &vec, const class Matrix3 &mat, float w = 1.0f);

		static const Vector2f Zero;
		static const Vector2f UnitX;
		static const Vector2f UnitY;
		static const Vector2f NegUnitX;
		static const Vector2f NegUnitY;
	};
	class Vector2i
	{
	public:
		int x;
		int y;

		Vector2i() : x(0), y(0)
		{
		}

		constexpr explicit Vector2i(int inX, int inY) : x(inX), y(inY)
		{
		}

		// Set both components in one line
		void Set(int inX, int inY)
		{
			x = inX;
			y = inY;
		}

		const int *GetAsFloatPtr() const
		{
			return reinterpret_cast<const int *>(&x);
		}
		// Vector addition (a + b)
		friend Vector2i operator+(const Vector2i &a, const Vector2i &b)
		{
			return Vector2i(a.x + b.x, a.y + b.y);
		}

		// Vector subtraction (a - b)
		friend Vector2i operator-(const Vector2i &a, const Vector2i &b)
		{
			return Vector2i(a.x - b.x, a.y - b.y);
		}

		// Component-wise multiplication
		// (a.x * b.x, ...)
		friend Vector2i operator*(const Vector2i &a, const Vector2i &b)
		{
			return Vector2i(a.x * b.x, a.y * b.y);
		}

		// Scalar multiplication
		friend Vector2i operator*(const Vector2i &vec, int scalar)
		{
			return Vector2i(vec.x * scalar, vec.y * scalar);
		}

		// Scalar multiplication
		friend Vector2i operator*(int scalar, const Vector2i &vec)
		{
			return Vector2i(vec.x * scalar, vec.y * scalar);
		}

		Vector2i &operator/=(int scalar)
		{
			x > 0 ? x /= scalar : x = 0;
			y > 0 ? y /= scalar : y = 0;
			return *this;
		}

		// Scalar *=
		Vector2i &operator*=(int scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		// Vector +=
		Vector2i &operator+=(const Vector2i &right)
		{
			x += right.x;
			y += right.y;
			return *this;
		}

		// Vector -=
		Vector2i &operator-=(const Vector2i &right)
		{
			x -= right.x;
			y -= right.y;
			return *this;
		}

		// Length squared of vector
		[[nodiscard]] float LengthSq() const
		{
			return static_cast<float>(x * x + y * y);
		}

		// Length of vector
		[[nodiscard]] float Length() const
		{
			return (Math::Sqrt(LengthSq()));
		}

		// Normalize this vector
		void Normalize()
		{
			const auto length = static_cast<int>(Length());
			x /= length;
			y /= length;
		}

		// Normalize the provided vector
		static Vector2i Normalize(const Vector2i &vec)
		{
			auto temp = vec;
			temp.Normalize();
			return temp;
		}

		// Dot product between two vectors (a dot b)
		static int Dot(const Vector2i &a, const Vector2i &b)
		{
			return (a.x * b.x + a.y * b.y);
		}

		// Lerp from A to B by f
		static Vector2i Lerp(const Vector2i &a, const Vector2i &b, int f)
		{
			return Vector2i(a + f * (b - a));
		}

		// Reflect V about (normalized) N
		static Vector2i Reflect(const Vector2i &v, const Vector2i &n)
		{
			return v - 2 * Vector2i::Dot(v, n) * n;
		}
	};

	// 3D Vector
	class Vector3f
	{
	public:
		float x;
		float y;
		float z;

		constexpr Vector3f()
			: x(0.0f), y(0.0f), z(0.0f)
		{
		}
		constexpr explicit Vector3f(float value)
			: x(value), y(value), z(value)
		{
		}

		constexpr explicit Vector3f(float inX, float inY, float inZ)
			: x(inX), y(inY), z(inZ)
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
		friend Vector3f operator+(const Vector3f &a, const Vector3f &b)
		{
			return Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
		}

		// Vector subtraction (a - b)
		friend Vector3f operator-(const Vector3f &a, const Vector3f &b)
		{
			return Vector3f(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		// Component-wise multiplication
		friend Vector3f operator*(const Vector3f &left, const Vector3f &right)
		{
			return Vector3f(left.x * right.x, left.y * right.y, left.z * right.z);
		}

		// Scalar multiplication
		friend Vector3f operator*(const Vector3f &vec, float scalar)
		{
			return Vector3f(vec.x * scalar, vec.y * scalar, vec.z * scalar);
		}

		// Scalar multiplication
		friend Vector3f operator*(float scalar, const Vector3f &vec)
		{
			return Vector3f(vec.x * scalar, vec.y * scalar, vec.z * scalar);
		}

		// Scalar *=
		Vector3f &operator*=(float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		// Vector +=
		Vector3f &operator+=(const Vector3f &right)
		{
			x += right.x;
			y += right.y;
			z += right.z;
			return *this;
		}

		// Vector -=
		Vector3f &operator-=(const Vector3f &right)
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
		static Vector3f Normalize(const Vector3f &vec)
		{
			auto temp = vec;
			temp.Normalize();
			return temp;
		}

		// Dot product between two vectors (a dot b)
		static float Dot(const Vector3f &a, const Vector3f &b)
		{
			return (a.x * b.x + a.y * b.y + a.z * b.z);
		}

		// Cross product between two vectors (a cross b)
		static Vector3f Cross(const Vector3f &a, const Vector3f &b)
		{
			Vector3f temp;
			temp.x = a.y * b.z - a.z * b.y;
			temp.y = a.z * b.x - a.x * b.z;
			temp.z = a.x * b.y - a.y * b.x;
			return temp;
		}

		// Lerp from A to B by f
		static Vector3f Lerp(const Vector3f &a, const Vector3f &b, float f)
		{
			return Vector3f(a + f * (b - a));
		}

		// Reflect V about (normalized) N
		static Vector3f Reflect(const Vector3f &v, const Vector3f &n)
		{
			return v - 2.0f * Vector3f::Dot(v, n) * n;
		}

		static Vector3f Transform(const Vector3f &vec, const class Matrix4 &mat, float w = 1.0f);
		// This will transform the vector and renormalize the w component
		static Vector3f TransformWithPerspDiv(const Vector3f &vec, const class Matrix4 &mat, float w = 1.0f);

		// Transform a Vector3f by a quaternion
		static Vector3f Transform(const Vector3f &v, const class Quaternion &q);

		/// <summary>
		/// VDE Vector3f to Effekseer Vector3D
		/// </summary>
		//Effekseer::Vector3D ToEffekseer() const;

		static const Vector3f Zero;
		static const Vector3f UnitX;
		static const Vector3f UnitY;
		static const Vector3f UnitZ;
		static const Vector3f NegUnitX;
		static const Vector3f NegUnitY;
		static const Vector3f NegUnitZ;
		static const Vector3f Infinity;
		static const Vector3f NegInfinity;
	};

	// 3x3 Matrix
	class Matrix3
	{
	public:
		float mat[3][3] = {};

		Matrix3()
		{
			*this = Matrix3::Identity;
		}

		explicit Matrix3(float inMat[3][3])
		{
			memcpy(mat, inMat, 9 * sizeof(float));
		}

		// Cast to a const float pointer
		[[nodiscard]] const float *GetAsFloatPtr() const
		{
			return reinterpret_cast<const float *>(&mat[0][0]);
		}

		// Matrix multiplication
		friend Matrix3 operator*(const Matrix3 &left, const Matrix3 &right)
		{
			Matrix3 retVal;
			// row 0
			retVal.mat[0][0] =
				left.mat[0][0] * right.mat[0][0] +
				left.mat[0][1] * right.mat[1][0] +
				left.mat[0][2] * right.mat[2][0];

			retVal.mat[0][1] =
				left.mat[0][0] * right.mat[0][1] +
				left.mat[0][1] * right.mat[1][1] +
				left.mat[0][2] * right.mat[2][1];

			retVal.mat[0][2] =
				left.mat[0][0] * right.mat[0][2] +
				left.mat[0][1] * right.mat[1][2] +
				left.mat[0][2] * right.mat[2][2];

			// row 1
			retVal.mat[1][0] =
				left.mat[1][0] * right.mat[0][0] +
				left.mat[1][1] * right.mat[1][0] +
				left.mat[1][2] * right.mat[2][0];

			retVal.mat[1][1] =
				left.mat[1][0] * right.mat[0][1] +
				left.mat[1][1] * right.mat[1][1] +
				left.mat[1][2] * right.mat[2][1];

			retVal.mat[1][2] =
				left.mat[1][0] * right.mat[0][2] +
				left.mat[1][1] * right.mat[1][2] +
				left.mat[1][2] * right.mat[2][2];

			// row 2
			retVal.mat[2][0] =
				left.mat[2][0] * right.mat[0][0] +
				left.mat[2][1] * right.mat[1][0] +
				left.mat[2][2] * right.mat[2][0];

			retVal.mat[2][1] =
				left.mat[2][0] * right.mat[0][1] +
				left.mat[2][1] * right.mat[1][1] +
				left.mat[2][2] * right.mat[2][1];

			retVal.mat[2][2] =
				left.mat[2][0] * right.mat[0][2] +
				left.mat[2][1] * right.mat[1][2] +
				left.mat[2][2] * right.mat[2][2];

			return retVal;
		}

		Matrix3 &operator*=(const Matrix3 &right)
		{
			*this = *this * right;
			return *this;
		}

		// Create a scale matrix with x and y scales
		static Matrix3 CreateScale(float xScale, float yScale)
		{
			float temp[3][3] =
				{
					{xScale, 0.0f, 0.0f},
					{0.0f, yScale, 0.0f},
					{0.0f, 0.0f, 1.0f},
				};
			return Matrix3(temp);
		}

		static Matrix3 CreateScale(const Vector2f &scaleVector)
		{
			return CreateScale(scaleVector.x, scaleVector.y);
		}

		// Create a scale matrix with a uniform factor
		static Matrix3 CreateScale(float scale)
		{
			return CreateScale(scale, scale);
		}

		// Create a rotation matrix about the Z axis
		// theta is in radians
		static Matrix3 CreateRotation(float theta)
		{
			float temp[3][3] =
				{
					{Math::Cos(theta), Math::Sin(theta), 0.0f},
					{-Math::Sin(theta), Math::Cos(theta), 0.0f},
					{0.0f, 0.0f, 1.0f},
				};
			return Matrix3(temp);
		}

		// Create a translation matrix (on the xy-plane)
		static Matrix3 CreateTranslation(const Vector2f &trans)
		{
			float temp[3][3] =
				{
					{1.0f, 0.0f, 0.0f},
					{0.0f, 1.0f, 0.0f},
					{trans.x, trans.y, 1.0f},
				};
			return Matrix3(temp);
		}

		static const Matrix3 Identity;
	};

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
		[[nodiscard]] Vector3f GetTranslation() const
		{
			return Vector3f(mat[3][0], mat[3][1], mat[3][2]);
		}

		// Get the X axis of the matrix (forward)
		[[nodiscard]] Vector3f GetXAxis() const
		{
			return Vector3f::Normalize(Vector3f(mat[0][0], mat[0][1], mat[0][2]));
		}

		// Get the Y axis of the matrix (left)
		[[nodiscard]] Vector3f GetYAxis() const
		{
			return Vector3f::Normalize(Vector3f(mat[1][0], mat[1][1], mat[1][2]));
		}

		// Get the Z axis of the matrix (up)
		[[nodiscard]] Vector3f GetZAxis() const
		{
			return Vector3f::Normalize(Vector3f(mat[2][0], mat[2][1], mat[2][2]));
		}

		// Extract the scale component from the matrix
		[[nodiscard]] Vector3f GetScale() const
		{
			Vector3f retVal;
			retVal.x = Vector3f(mat[0][0], mat[0][1], mat[0][2]).Length();
			retVal.y = Vector3f(mat[1][0], mat[1][1], mat[1][2]).Length();
			retVal.z = Vector3f(mat[2][0], mat[2][1], mat[2][2]).Length();
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

		static const Matrix4 Identity;
	};

	// (Unit) Quaternion
	class Quaternion
	{
	public:
		float x{};
		float y{};
		float z{};
		float w{};

		Quaternion()
		{
			*this = Quaternion::Identity;
		}

		explicit Quaternion(float inX, float inY, float inZ, float inW)
		{
			Set(inX, inY, inZ, inW);
		}

		explicit Quaternion(const Vector3f &axis, float angle)
		{
			const auto scalar = Math::Sin(angle / 2.0f);
			x = axis.x * scalar;
			y = axis.y * scalar;
			z = axis.z * scalar;
			w = Math::Cos(angle / 2.0f);
		}

		// Directly set the internal components
		void Set(float inX, float inY, float inZ, float inW)
		{
			x = inX;
			y = inY;
			z = inZ;
			w = inW;
		}

		void Conjugate()
		{
			x *= -1.0f;
			y *= -1.0f;
			z *= -1.0f;
		}

		[[nodiscard]] float LengthSq() const
		{
			return (x * x + y * y + z * z + w * w);
		}

		[[nodiscard]] float Length() const
		{
			return Math::Sqrt(LengthSq());
		}

		void Normalize()
		{
			const auto length = Length();
			x /= length;
			y /= length;
			z /= length;
			w /= length;
		}

		// Normalize the provided quaternion
		static Quaternion Normalize(const Quaternion &q)
		{
			auto retVal = q;
			retVal.Normalize();
			return retVal;
		}

		// Linear interpolation
		static Quaternion Lerp(const Quaternion &a, const Quaternion &b, float f)
		{
			Quaternion retVal;
			retVal.x = Math::Lerp(a.x, b.x, f);
			retVal.y = Math::Lerp(a.y, b.y, f);
			retVal.z = Math::Lerp(a.z, b.z, f);
			retVal.w = Math::Lerp(a.w, b.w, f);
			retVal.Normalize();
			return retVal;
		}

		static float Dot(const Quaternion &a, const Quaternion &b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		// Spherical Linear Interpolation
		static Quaternion Slerp(const Quaternion &a, const Quaternion &b, float f)
		{
			const auto rawCosm = Quaternion::Dot(a, b);

			auto cosom = -rawCosm;
			if (rawCosm >= 0.0f)
			{
				cosom = rawCosm;
			}

			float scale0, scale1;

			if (cosom < 0.9999f)
			{
				const auto omega = Math::Acos(cosom);
				const auto invSin = 1.f / Math::Sin(omega);
				scale0 = Math::Sin((1.f - f) * omega) * invSin;
				scale1 = Math::Sin(f * omega) * invSin;
			}
			else
			{
				scale0 = 1.0f - f;
				scale1 = f;
			}

			if (rawCosm < 0.0f)
			{
				scale1 = -scale1;
			}

			Quaternion retVal;
			retVal.x = scale0 * a.x + scale1 * b.x;
			retVal.y = scale0 * a.y + scale1 * b.y;
			retVal.z = scale0 * a.z + scale1 * b.z;
			retVal.w = scale0 * a.w + scale1 * b.w;
			retVal.Normalize();
			return retVal;
		}

		// Concatenate
		// Rotate by q FOLLOWED BY p
		static Quaternion Concatenate(const Quaternion &q, const Quaternion &p)
		{
			Quaternion retVal;

			const Vector3f qv(q.x, q.y, q.z);
			const Vector3f pv(p.x, p.y, p.z);
			const auto newVec = p.w * qv + q.w * pv + Vector3f::Cross(pv, qv);
			retVal.x = newVec.x;
			retVal.y = newVec.y;
			retVal.z = newVec.z;

			retVal.w = p.w * q.w - Vector3f::Dot(pv, qv);

			return retVal;
		}

		static const Quaternion Identity;
	};

	namespace Color
	{
		class Color
		{
		public:
			Color(const float &red, const float &green, const float &blue, const float &alpha)
			{
				r = red;
				g = green;
				b = blue;
				a = alpha;
			}

			float r;
			float g;
			float b;
			float a;
		};

		static constexpr Vector3f Black(0.0f, 0.0f, 0.0f);
		static constexpr Vector3f LightBlack(0.2f, 0.2f, 0.2f);
		static constexpr Vector3f White(1.0f, 1.0f, 1.0f);
		static constexpr Vector3f Red(1.0f, 0.0f, 0.0f);
		static constexpr Vector3f Green(0.0f, 1.0f, 0.0f);
		static constexpr Vector3f Blue(0.0f, 0.0f, 1.0f);
		static constexpr Vector3f Yellow(1.0f, 1.0f, 0.0f);
		static constexpr Vector3f LightYellow(1.0f, 1.0f, 0.88f);
		static constexpr Vector3f LightBlue(0.68f, 0.85f, 0.9f);
		static constexpr Vector3f LightPink(1.0f, 0.71f, 0.76f);
		static constexpr Vector3f LightGreen(0.56f, 0.93f, 0.56f);
	} // namespace Color
}