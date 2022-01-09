#include <Nen.hpp>
#include <iostream>
namespace nen {
text_component::text_component(base_actor &actor, int drawOrder)
    : draw_2d_component(actor, drawOrder) {}

void text_component::SetString(const std::string &str, const color &color) {
  if (!mFont->isLoaded()) {
    mFont->LoadFromFile("Assets/Font/mplus/mplus-1p-medium.ttf", 32);
  }

  if (this->GetSprite()) {
    GetActor().GetScene().GetRenderer()->RemoveDrawObject2D(this->GetSprite());
    this->GetSprite().reset();
  }
  Create(mFont->RenderText(str, color));
}
void text_component::SetFont(std::shared_ptr<font> font) { mFont = font; }
} // namespace nen