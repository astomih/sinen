#pragma once
#include "UIScreen.hpp"
#include <array>
#include <vector>

namespace nen {
class hud : public ui_screen {
public:
  hud();
  void Update(float deltaTime) override;
  void HandleInput(const input_state &state) override;
};
} // namespace nen