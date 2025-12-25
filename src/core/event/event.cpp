#include "event.hpp"
#include "event_system.hpp"

namespace sinen {
void Event::quit() { EventSystem::setQuit(true); }
} // namespace sinen