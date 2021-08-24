#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <Nen.hpp>

namespace nen
{
	class GameController::Impl
	{
	public:
		Impl() :controller(nullptr) {}
		~Impl() = default;
		::SDL_GameController* controller;
	};
	GameController::GameController()
		: impl(nullptr) 
	{}

	GameController::~GameController() = default;

	bool GameController::Initialize()
	{
		impl = std::make_unique<GameController::Impl>();
		impl->controller = SDL_GameControllerOpen(0);
		if (impl->controller)return true;
		return false;
	}

	int16_t GameController::GetAxis(GameController::Axis axis)
	{
		return ::SDL_GameControllerGetAxis(this->impl->controller, static_cast<SDL_GameControllerAxis>(axis));
	}
	uint8_t GameController::GetButton(GameControllerButton button)
	{
		return SDL_GameControllerGetButton(impl->controller, static_cast<SDL_GameControllerButton>(button));
	}
}