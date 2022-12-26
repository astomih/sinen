#include <component/rigidbody_component.hpp>

namespace sinen {
rigidbody_component::rigidbody_component(actor &owner) : component(owner) {}
rigidbody_component::~rigidbody_component() {}
void rigidbody_component::update(float delta_time) {}
} // namespace sinen