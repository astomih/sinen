#include "scene_system.hpp"
#include <scene/scene.hpp>

namespace sinen {

void scene::reset() {
  scene_system::shutdown();
  scene_system::set_state(scene::state::quit);
}

bool scene::is_running() { return scene_system::is_running(); }

void scene::set_state(const scene::state &state) {
  scene_system::set_state(state);
}
const scene::state &scene::get_state() { return scene_system::get_state(); }

void scene::change_impl(std::unique_ptr<scene::implements> impl) {
  scene_system::change_impl(std::move(impl));
}

} // namespace sinen
