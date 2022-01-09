#pragma once
#include "../Color/Color.hpp"
#include "../Math/Vector3.hpp"
#include "Draw2DComponent.hpp"
#include <memory>
#include <string>

namespace nen {
class text_component : public draw_2d_component {
public:
  text_component(class base_actor &, int drawOrder = 10000);
  void SetString(const std::string &,
                 const class color &color = palette::White);
  void SetFont(std::shared_ptr<class font> font);

private:
  std::shared_ptr<class font> mFont;
};
} // namespace nen