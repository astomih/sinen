#include <Engine.hpp>
template <typename T>
void vde::console::Print(const T& t)
{
	std::cout << t << std::endl;
}
template <>
void vde::console::Print(const Vector3f& t)
{
	std::cout << t.x << "," << t.y << "," << t.z << std::endl;
}
template <>
void vde::console::Print(const Vector2f& t)
{
	std::cout << t.x << "," << t.y << std::endl;
}
template <>
void vde::console::Print(const Matrix4& t)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << t.GetAsFloatPtr()[i * 4 + j] << ",";
		}
		std::cout << std::endl;
	}
}