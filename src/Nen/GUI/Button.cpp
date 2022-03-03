#include <GUI/Button.hpp>
namespace nen {
button::button(const std::string &name, std::shared_ptr<font> font,
               std::function<void()> onClick, const vector2 &pos,
               const vector2 &dims)
    : mOnClick(onClick), mNameTex(nullptr), mFont(font), mPosition(pos),
      mDimensions(dims), mHighlighted(false) {
  SetName(name);
}

button::~button() {}

void button::SetName(const std::string &name) { mName = name; }

bool button::ContainsPoint(const vector2 &pt) const {
  const bool no = pt.x < (mPosition.x - mDimensions.x / 2.0f) ||
                  pt.x > (mPosition.x + mDimensions.x / 2.0f) ||
                  pt.y < (mPosition.y - mDimensions.y / 2.0f) ||
                  pt.y > (mPosition.y + mDimensions.y / 2.0f);
  return no;
}

void button::OnClick() {
  if (mOnClick) {
    mOnClick();
  }
}
} // namespace nen