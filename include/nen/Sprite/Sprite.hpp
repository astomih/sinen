#pragma once
#include "../Math/Vector2.hpp"
namespace nen
{
	class Sprite
	{
	public:
		Sprite() : trimStart(Vector2(0.f, 1.f)), trimEnd(Vector2(1.f, 0.f)) {}
		~Sprite() {}
		ShaderParameters param;
		std::string textureIndex;
		std::string vertexIndex;
		Vector2 trimStart;
		Vector2 trimEnd;
		int drawOrder = 100;
		bool isChangeBuffer = false;
	};

}
