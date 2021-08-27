#pragma once
#include "../Math/Vector2.hpp"
namespace nen
{
	class DrawObject
	{
	public:
		DrawObject() = default;
		~DrawObject() = default;
		ShaderParameters param;
		std::string textureIndex;
		std::string vertexIndex;
		int drawOrder = 100;
	};

}
