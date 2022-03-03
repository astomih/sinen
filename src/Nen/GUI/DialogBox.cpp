#include <GUI/DialogBox.hpp>
namespace nen {
dialog_box::dialog_box(std::string_view text, std::function<void()> onOK)
    : ui_screen() {
  // Adjust positions for dialog box
  mBGPos = vector2(0.0f, 0.0f);
  mTitlePos = vector2(0.0f, 100.0f);
  mNextButtonPos = vector2(0.0f, 0.0f);
  SetTitle(text.data(), palette::Black, 30);
  AddButton("OK", [onOK]() { onOK(); });
  AddButton("Cancel", [this]() { Close(); });
}

dialog_box::~dialog_box() {}
} // namespace nen