#pragma once
#include "../Math/Vector2.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
namespace nen
{
	class Window
	{
	public:
		struct Info
		{
			static Uint32 id;
			static ::SDL_SysWMinfo info;
		};
		static constexpr float WINDOW_SIZE_X = 1024.f;
		static constexpr float WINDOW_SIZE_Y = 768.f;
		static constexpr Vector2 Size = Vector2(WINDOW_SIZE_X, WINDOW_SIZE_Y);
		static constexpr Vector2 Center = Vector2(0.f, 0.f);
		static inline std::string name = std::string("");
	};
}