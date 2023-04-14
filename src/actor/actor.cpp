// internal
#include <actor/actor.hpp>
#include <scene/scene.hpp>

namespace sinen {
actor::actor()
    : m_state(actor::state::active), m_name("actor"),
      m_script_name("default.lua") {}
actor::~actor() {}
void actor::update(float delta_time) {
  for (auto &c : m_components) {
    c->update(delta_time);
  }
}
matrix4 actor::get_world_matrix() const {
  return matrix4::create_scale(m_scale) *
         matrix4::create_from_quaternion(quaternion::from_euler(m_rotation)) *
         matrix4::create_translation(m_position);
}
void actor::add_component(actor::component_ptr comp) {
  if (!has_component(comp)) {
    m_components.push_back(std::move(comp));
  }
}
void actor::add_component(std::string_view name) {
  if (!has_component(name)) {
    this->add_component(
        scene::get_component_factory().create(name.data(), *this));
  }
}
component *actor::get_component(std::string_view name) {
  for (auto &c : m_components) {
    if (c->get_name() == std::string(name)) {
      return c.get();
    }
  }
  return nullptr;
}
void actor::remove_component(actor::component_ptr comp) {
  for (auto itr = m_components.begin(); itr != m_components.end();) {
    if (*itr == comp) {
      m_components.erase(itr);
      break;
    }
    itr++;
  }
}
void actor::remove_component(std::string_view name) {
  for (auto itr = m_components.begin(); itr != m_components.end();) {
    if ((*itr)->get_name() == std::string(name)) {
      m_components.erase(itr);
      break;
    }
    itr++;
  }
}
bool actor::has_component(component_ptr comp) const {
  for (auto &c : m_components) {
    if (c->get_name() == comp->get_name()) {
      return true;
    }
  }
  return false;
}
bool actor::has_component(std::string_view name) const {
  for (auto &c : m_components) {
    if (c->get_name() == std::string(name)) {
      return true;
    }
  }
  return false;
}
} // namespace sinen
