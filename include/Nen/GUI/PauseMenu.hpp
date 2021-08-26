#pragma once
#include "UIScreen.hpp"
namespace nen
{
	class PauseMenu : public UIScreen
	{
	public:
		PauseMenu();
		~PauseMenu();

		void HandleInput(const InputState &state) override;
	};
}