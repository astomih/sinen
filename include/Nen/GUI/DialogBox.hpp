#pragma once
#include "UIScreen.hpp"
#ifdef DialogBox
#undef DialogBox
#endif

namespace nen {
class dialog_box : public ui_screen {
public:
  dialog_box(std::string_view, std::function<void()> onOK);
  ~dialog_box();
};
} // namespace nen