#pragma once
#include "../Math/Vector2.hpp"
#include "ObjectType.hpp"
#include "../Shader/Shader.hpp"
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
		Shader shader;
		int drawOrder = 100;
	};

}
