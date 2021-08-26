#include <Nen.hpp>
namespace nen
{
	PauseMenu::PauseMenu()
		: UIScreen()
	{
		SetRelativeMouseMode(false);
		SetTitle("Pause");
		AddButton("Resume", [this]()
				  { Close(); });
		AddButton("Quit", [this]() {

		});
	}

	PauseMenu::~PauseMenu()
	{
		SetRelativeMouseMode(true);
	}

	void PauseMenu::HandleInput(const InputState &state)
	{
	}

}