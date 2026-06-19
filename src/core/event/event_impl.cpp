#include "event_impl.hpp"

namespace sinen {
bool Event::quitRequested = false;
bool Event::paused = false;

void Event::setQuit(bool quit) { quitRequested = quit; }
bool Event::isQuit() { return quitRequested; }
bool Event::isPaused() { return paused; }
void Event::setPaused(bool value) { paused = value; }

Ptr<Event> createEvent(const SDL_Event &event) {
  return makePtr<EventImpl>(GlobalAllocator::get(), event);
}

EventImpl::EventImpl(const SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    eventType = Type::Quit;
    break;
  case SDL_EVENT_DID_ENTER_BACKGROUND:
    eventType = Type::DidEnterBackground;
    break;
  case SDL_EVENT_WILL_ENTER_FOREGROUND:
    eventType = Type::WillEnterForeground;
    break;
  case SDL_EVENT_WINDOW_RESIZED:
    eventType = Type::WindowResized;
    break;
  case SDL_EVENT_MOUSE_WHEEL:
    eventType = Type::MouseWheel;
    wheelData = {event.wheel.x, event.wheel.y};
    break;
  case SDL_EVENT_KEY_DOWN:
    eventType = Type::KeyDown;
    key = event.key.key;
    break;
  case SDL_EVENT_TEXT_INPUT:
    eventType = Type::TextInput;
    break;
  default:
    break;
  }
}

void EventImpl::processEvent() {
  switch (eventType) {
  case Type::Quit:
    setQuit(true);
    break;
  case Type::DidEnterBackground:
    setPaused(true);
    break;
  case Type::WillEnterForeground:
    setPaused(false);
    break;
  default:
    break;
  }
}

Event::Type EventImpl::type() const { return eventType; }

Event::MouseWheelData EventImpl::mouseWheel() const { return wheelData; }

std::uint32_t EventImpl::keyCode() const { return key; }
} // namespace sinen
