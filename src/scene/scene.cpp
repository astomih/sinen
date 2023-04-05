#include "scene_system.hpp"
#include <window/window.hpp>

namespace sinen {
camera scene::m_main_camera = []() {
  camera c;
  c.lookat(vector3{0, -1, 1}, vector3{0, 0, 0}, vector3{0, 0, 1});
  c.perspective(70.f, window::size().x / window::size().y, .1f, 1000.f);
  return c;
}();
component_factory scene::m_component_factory;
vector2 scene::m_screen_size = vector2(1280.0, 720.0);

void scene::reset() {
  scene_system::shutdown();
  scene_system::set_state(scene::state::quit);
}

bool scene::is_running() { return scene_system::is_running(); }

void scene::set_state(const scene::state &state) {
  scene_system::set_state(state);
}
const scene::state &scene::get_state() { return scene_system::get_state(); }
void scene::change(const std::string &scene_file_name) {
  scene_system::change(scene_file_name);
}
std::string scene::current_name() { return scene_system::current_name(); }
void scene::change_impl(std::unique_ptr<scene::implements> impl) {
  scene_system::change_impl(std::move(impl));
}
void scene::set_run_script(bool is_run) {
  scene_system::set_run_script(is_run);
}
void scene::add_actor(actor *_actor) { scene_system::add_actor(_actor); }
actor &scene::get_actor(const std::string &name) {
  return scene_system::get_actor(name);
}
void scene::load(std::string_view data_file_name) {
  scene_system::load_data(data_file_name);
}

} // namespace sinen
