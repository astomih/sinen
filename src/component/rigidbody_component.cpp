// internal
#include <actor/actor.hpp>
#include <component/rigidbody_component.hpp>
namespace sinen {
rigidbody_component::rigidbody_component(Actor &owner) : component(owner) {}
rigidbody_component::~rigidbody_component() {}
void rigidbody_component::update(float delta_time) {
  m_owner.set_position(m_owner.get_position() +
                       Vector3(0.f, 0.f, 9.8f) * delta_time);
}
} // namespace sinen