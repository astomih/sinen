#include <actor/actor.hpp>
#include <scene/scene.hpp>

namespace sinen {
actor::actor(scene &scene) : m_scene(scene) {}
actor::~actor() {}
void actor::update(float delta_time) {}
void actor::update_components(float delta_time) {
  for (auto &c : m_components.data) {
    c.second->update(delta_time);
  }
}
matrix4 actor::get_world_matrix() const {
  matrix4 s;
  s[0][0] = m_scale.x;
  s[1][1] = m_scale.y;
  s[2][2] = m_scale.z;
  return s * matrix4::create_from_quaternion(m_rotation) *
         matrix4::create_translation(m_position);
}
} // namespace sinen