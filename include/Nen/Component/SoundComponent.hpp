#pragma once
#include "../Audio/SoundEvent.hpp"
#include "Component.hpp"
#include <string>
#include <vector>

namespace nen {
class sound_component : public base_component {
public:
  sound_component(class base_actor &owner, int updateOrder = 200);
  ~sound_component();

  void Update(float deltaTime) override;
  void OnUpdateWorldTransform() override;
  void StopAllEvents();
  void AddEvent(const sound_event &e);

private:
  std::vector<sound_event> mEvents;
};
} // namespace nen