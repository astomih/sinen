#pragma once
namespace nen
{
	class Sprite
	{
	public:
		Sprite() : trimStart(Vector2f(0.f, 1.f)), trimEnd(Vector2f(1.f, 0.f)) {}
		~Sprite() {}
		ShaderParameters param;
		std::string textureIndex;
		std::string vertexIndex;
		Vector2f trimStart;
		Vector2f trimEnd;
		int drawOrder = 100;
		bool isChangeBuffer = false;
	};
	
}
