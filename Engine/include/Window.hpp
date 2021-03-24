#pragma once
#include "Math.hpp"
#include <SDL.h>
#include <SDL_syswm.h>
namespace nen
{
	constexpr auto WINDOW_SIZE_X = 1280.f;
	constexpr auto WINDOW_SIZE_Y = 720.f;

	namespace Window
	{
		struct Info
		{
			static Uint32 id;
			static ::SDL_SysWMinfo info;
		};
		inline constexpr Vector2f Size(WINDOW_SIZE_X, WINDOW_SIZE_Y);
		inline constexpr Vector2f Center(0.f, 0.f);
		const std::string name = "Works";
	}
}