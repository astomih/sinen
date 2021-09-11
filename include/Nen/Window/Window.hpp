#pragma once
#include "../Math/Vector2.hpp"
#include "../Render/Renderer.hpp"
#include "WindowState.hpp"
#include <string>
#include <memory>
namespace
{
	struct SDL_Window;
}

namespace nen
{
	class Window
	{
	public:
		Window();
		~Window();

		void Initialize(const Vector2 &size, const std::string &name, GraphicsAPI api);

		::SDL_Window *GetSDLWindow();

		Vector2 Size() { return size; }
		std::string Name() { return name; }

		void ProcessInput(union SDL_Event &event);
		const WindowState &GetState() { return state; }

	private:
		Vector2 size;
		std::string name;
		class Impl;
		std::unique_ptr<Impl> impl;
		WindowState state = WindowState::ENTER;
	};
}