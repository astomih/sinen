#ifndef SINEN_EVENT_IMPL_HPP
#define SINEN_EVENT_IMPL_HPP

#include <core/data/ptr.hpp>
#include <core/event/event.hpp>

#include <SDL3/SDL_events.h>

namespace sinen {
class EventImpl : public Event {
public:
  explicit EventImpl(const SDL_Event &event);

  void processEvent() override;
  Type type() const override;
  MouseWheelData mouseWheel() const override;
  std::uint32_t keyCode() const override;

private:
  Type eventType = Type::Unknown;
  MouseWheelData wheelData{};
  std::uint32_t key = 0;
};
Ptr<Event> createEvent(const SDL_Event &event);
} // namespace sinen

#endif // SINEN_EVENT_IMPL_HPP
