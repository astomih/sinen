#pragma once
#include "../Time/Timer.hpp"
#include <memory>
#include <string>

namespace nen {
class effect_component : public base_component {
public:
  effect_component(class base_actor &actor, int updateOrder = 100);
  ~effect_component();

  void Create(std::string_view filePath);
  void Update(float deltaTime) override;

  void SetLoop(bool loop, timer interval);
  bool isLoop();

private:
  base_actor &mOwner;
  std::shared_ptr<class effect> mEffect;
};
} // namespace nen
