// internal
#include <actor/actor.hpp>
#include <component/move_component.hpp>
#include <input/keyboard.hpp>

namespace sinen {
move_component::move_component(actor &owner) : component(owner) {}
void move_component::update(float delta_time) {
  auto pos = get_actor().get_position();
  if (keyboard::is_down(keyboard::code::UP)) {
    pos.z += 1.0f * delta_time;
  }
  if (keyboard::is_down(keyboard::code::DOWN)) {
    pos.z -= 1.0f * delta_time;
  }
  if (keyboard::is_down(keyboard::code::LEFT)) {
    pos.x -= 1.0f * delta_time;
  }
  if (keyboard::is_down(keyboard::code::RIGHT)) {
    pos.x += 1.0f * delta_time;
  }
  get_actor().set_position(pos);
}

} // namespace sinen