#pragma once
#include "UIScreen.hpp"
#include <vector>
#include <array>

namespace nen
{
	class HUD : public UIScreen
	{
	public:
		HUD();
		void Update(float deltaTime) override;
		void HandleInput(const InputState &state) override;
	};
}