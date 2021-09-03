#pragma once
#include "../Math/Vector2.hpp"
namespace nen
{
	class Window
	{
	public:
		static constexpr float WINDOW_SIZE_X = 1280.f;
		static constexpr float WINDOW_SIZE_Y = 720.f;
		static constexpr Vector2 Size = Vector2(WINDOW_SIZE_X, WINDOW_SIZE_Y);
		static constexpr Vector2 Center = Vector2(0.f, 0.f);
		static inline std::string name = std::string("");
	};
}