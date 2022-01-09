#include <Nen.hpp>
namespace nen {
sound_component::sound_component(base_actor &owner, int updateOrder)
    : base_component(owner, updateOrder) {}

sound_component::~sound_component() { StopAllEvents(); }

void sound_component::Update(float deltaTime) {
  for (auto &i : mEvents) {
    i.SetPosition(mOwner.GetPosition());
  }
}

void sound_component::OnUpdateWorldTransform() {
  // Update 3D events' world transforms
  matrix4 world = mOwner.GetWorldTransform();
  for (auto &event : mEvents) {
    if (event.IsValid()) {
    }
  }
}

void sound_component::StopAllEvents() {
  // Stop all sounds
  for (auto &e : mEvents) {
    e.Stop();
  }
  // Clear events
  mEvents.clear();
}

void sound_component::AddEvent(const sound_event &e) {
  mEvents.emplace_back(e);
}
} // namespace nen