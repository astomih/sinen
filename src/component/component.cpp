#include <component/component.hpp>

namespace sinen {
component::component(actor &owner) : m_owner(owner) {}
component::~component() {}
void component::update(float delta_time) {}
} // namespace sinen
