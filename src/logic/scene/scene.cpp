#include "scene_system.hpp"
#include <platform/window/window.hpp>

namespace sinen {
Camera Scene::m_main_camera = []() {
  Camera c;
  c.LookAt(glm::vec3{0, -1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1});
  c.Perspective(90.f, Window::Size().x / Window::Size().y, .1f, 100.f);
  return c;
}();
glm::vec2 Scene::m_screen_size = glm::vec2(1280.0, 720.0);

void Scene::Reset() {
  SceneSystem::shutdown();
  SceneSystem::set_state(Scene::state::quit);
}

bool Scene::IsRunning() { return SceneSystem::is_running(); }

void Scene::SetState(const Scene::state &state) {
  SceneSystem::set_state(state);
}
const Scene::state &Scene::GetState() { return SceneSystem::get_state(); }
void Scene::Change(const std::string &sceneFileName, const std::string &basePath) {
  SceneSystem::Change(sceneFileName, basePath);
}
std::string Scene::GetCurrentName() { return SceneSystem::GetCurrentName(); }
void Scene::change_impl(std::unique_ptr<Scene::implements> impl) {
  SceneSystem::change_impl(std::move(impl));
}
void Scene::set_run_script(bool is_run) { SceneSystem::set_run_script(is_run); }
glm::vec2 Scene::Ratio() {
  return glm::vec2(Window::Size().x / Scene::Size().x,
                   Window::Size().y / Scene::Size().y);
}
glm::vec2 Scene::InvRatio() {
  return glm::vec2(Scene::Size().x / Window::Size().x,
                   Scene::Size().y / Window::Size().y);
}

float Scene::DeltaTime() { return SceneSystem::delta_time(); }

} // namespace sinen
