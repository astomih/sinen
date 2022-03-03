#include <GUI/PauseMenu.hpp>
namespace nen {
pause_menu::pause_menu() : ui_screen() {
  SetRelativeMouseMode(false);
  SetTitle("Pause");
  AddButton("Resume", [this]() { Close(); });
  AddButton("Quit", [this]() {

  });
}

pause_menu::~pause_menu() { SetRelativeMouseMode(true); }

void pause_menu::HandleInput(const input_state &state) {}

} // namespace nen