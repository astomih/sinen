#pragma once
#include "UIScreen.hpp"
namespace nen {
class pause_menu : public ui_screen {
public:
  pause_menu();
  ~pause_menu();

  void HandleInput(const input_state &state) override;
};
} // namespace nen