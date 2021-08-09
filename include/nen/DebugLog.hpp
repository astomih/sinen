#pragma once
#include <string>

namespace nen::console
{
	template <typename T>
	void Print(const T &t);
	template <>
	void Print(const Vector3f &t);
	template <>
	void Print(const Vector2f &t);
	template <>
	void Print(const Matrix4 &t);
}
