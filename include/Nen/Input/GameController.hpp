#pragma once
#include <memory>
#include <cstdint>
#include "GameControllerButton.hpp"

namespace nen
{
	class GameController
	{
	public:
		GameController();
		~GameController();

		enum class Axis
		{
			INVALID = -1,
			LEFTX,
			LEFTY,
			RIGHTX,
			RIGHTY,
			TRIGGERLEFT,
			TRIGGERRIGHT,
			MAX
		};

		bool Initialize();
		int16_t GetAxis(Axis axis);
		uint8_t GetButton(GameControllerButton button);
	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
