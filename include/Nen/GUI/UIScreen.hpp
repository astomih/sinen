#pragma once
#include "../Color/Color.hpp"
#include "../Input/InputSystem.hpp"
#include "Button.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nen {
class ui_screen {
public:
  ui_screen();
  virtual ~ui_screen();
  // UIScreen subclasses can override these
  virtual void Update(float deltaTime);
  virtual void HandleInput(const input_state &state);
  // Tracks if the UI is active or closing
  enum class State { Active, Closing };
  // Set state to closing
  void Close();
  // Get state of UI screen
  State GetState() const { return mState; }
  // Change the title text
  void SetTitle(std::string_view text, const color &color = palette::White,
                int pointSize = 40);
  // Add a button to this screen
  void AddButton(std::string_view name, std::function<void()> onClick);

protected:
  // Helper to draw a texture
  void DrawTexture(class shader *shader, class texture *texture,
                   const vector2 &offset = vector2::Zero, float scale = 1.0f);
  // Sets the mouse mode to relative or not
  void SetRelativeMouseMode(bool relative);

  std::shared_ptr<class font> mFont;

  std::shared_ptr<class texture> mTitle;
  std::shared_ptr<class texture> mButtonOn;
  std::shared_ptr<class texture> mButtonOff;
  std::shared_ptr<class texture> mBackground;
  // Configure positions
  vector2 mTitlePos;
  vector2 mNextButtonPos;
  vector2 mBGPos;

  // State
  State mState;
  // List of buttons
  std::vector<std::unique_ptr<button>> mButtons;
};
} // namespace nen