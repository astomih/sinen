#include "math/vector2.hpp"
#include "scene_system.hpp"
#include <window/window.hpp>

namespace sinen {
Camera Scene::m_main_camera = []() {
  Camera c;
  c.lookat(Vector3{0, -1, 1}, Vector3{0, 0, 0}, Vector3{0, 0, 1});
  c.perspective(70.f, Window::size().x / Window::size().y, .1f, 1000.f);
  return c;
}();
Vector2 Scene::m_screen_size = Vector2(1280.0, 720.0);

void Scene::reset() {
  scene_system::shutdown();
  scene_system::set_state(Scene::state::quit);
}

bool Scene::is_running() { return scene_system::is_running(); }

void Scene::set_state(const Scene::state &state) {
  scene_system::set_state(state);
}
const Scene::state &Scene::get_state() { return scene_system::get_state(); }
void Scene::change(const std::string &scene_file_name) {
  scene_system::change(scene_file_name);
}
std::string Scene::current_name() { return scene_system::current_name(); }
void Scene::change_impl(std::unique_ptr<Scene::implements> impl) {
  scene_system::change_impl(std::move(impl));
}
void Scene::set_run_script(bool is_run) {
  scene_system::set_run_script(is_run);
}
Vector2 Scene::ratio() {
  return Vector2(Window::size().x / Scene::size().x,
                 Window::size().y / Scene::size().y);
}

float Scene::delta_time() { return scene_system::delta_time(); }

} // namespace sinen
