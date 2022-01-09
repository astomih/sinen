#pragma once
#include "Component.hpp"

namespace nen {
class billboard_component : public base_component {
public:
  billboard_component(class base_actor &owner, int updateOwner = 100);
  void Update(float deltaTime) override;
};
} // namespace nen