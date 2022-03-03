#include <GUI/UIScreen.hpp>
namespace nen {
ui_screen::ui_screen()
    : mTitle(nullptr), mBackground(nullptr), mTitlePos(0.0f, 300.0f),
      mNextButtonPos(0.0f, 200.0f), mBGPos(0.0f, 250.0f),
      mState(State::Active) {}

ui_screen::~ui_screen() { mButtons.clear(); }

void ui_screen::Update(float deltaTime) {}

void ui_screen::HandleInput(const input_state &state) {
  // Do we have buttons?
  if (!mButtons.empty()) {
    // Get position of mouse
    int x, y;
    x = state.Mouse.GetPosition().x;
    y = state.Mouse.GetPosition().y;
    // Convert to (0,0) center coordinates
    vector2 mousePos((x), (y));
    mousePos.x -= 1280 * 0.5f;
    mousePos.y = 720 * 0.5f - mousePos.y;

    // Highlight any buttons
    for (auto &b : mButtons) {
      if (b->ContainsPoint(mousePos)) {
        b->SetHighlighted(true);
      } else {
        b->SetHighlighted(false);
      }
    }
  }
}

void ui_screen::Close() { mState = State::Closing; }

void ui_screen::SetTitle(std::string_view text, const color &color,
                         int pointSize) {}

void ui_screen::AddButton(std::string_view name,
                          std::function<void()> onClick) {
  vector2 dims((mButtonOn->GetWidth()), (mButtonOn->GetHeight()));
  auto b = std::make_unique<button>(name.data(), mFont, onClick, mNextButtonPos,
                                    dims);
  mButtons.emplace_back(std::move(b));

  mNextButtonPos.y -= mButtonOff->GetHeight() + 20.0f;
}

void ui_screen::SetRelativeMouseMode(bool relative) {}

} // namespace nen