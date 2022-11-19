#include <actor/actor.hpp>
#include <scene/scene.hpp>

namespace sinen {
actor::actor() {}
actor::~actor() {}
void actor::update(float delta_time) {
  for (auto &c : m_components) {
    c.get().update(delta_time);
  }
}
matrix4 actor::get_world_matrix() const {
  matrix4 s;
  s[0][0] = m_scale.x;
  s[1][1] = m_scale.y;
  s[2][2] = m_scale.z;
  return s * // matrix4::create_from_quaternion(m_rotation) *
         matrix4::create_translation(m_position);
}

void actor::add_component(component &comp) { m_components.push_back(comp); }
void actor::remove_component(component &comp) {
  std::erase_if(m_components,
                [&comp](ref_component &c) { return &c.get() == &comp; });
}
} // namespace sinen