#include "scene_system.hpp"
#include <camera/camera.hpp>
#include <color/color.hpp>
#include <color/palette.hpp>
#include <io/data_stream.hpp>
#include <io/json.hpp>

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
void scene::set_run_script(bool is_run) {
  scene_system::set_run_script(is_run);
}
void scene::add_actor(actor &_actor) { scene_system::add_actor(_actor); }
void scene::remove_actor(actor &_actor) { scene_system::remove_actor(_actor); }
void scene::load_data(std::string_view data_file_name) {
  json doc;
  auto str = data_stream::open_as_string(asset_type::Scene, data_file_name);
  doc.parse(str.data());
  draw3d d;
  texture tex;
  tex.fill_color(palette::white());
  d.position.x = doc["Actors"]["px"].get_float();
  d.position.y = doc["Actors"]["py"].get_float();
  d.position.z = doc["Actors"]["pz"].get_float();
  d.rotation.x = doc["Actors"]["rx"].get_float();
  d.rotation.y = doc["Actors"]["ry"].get_float();
  d.rotation.z = doc["Actors"]["rz"].get_float();
  d.scale.x = doc["Actors"]["sx"].get_float();
  d.scale.y = doc["Actors"]["sy"].get_float();
  d.scale.z = doc["Actors"]["sz"].get_float();
  d.vertex_name = "BOX";
  d.texture_handle = tex;
  vector3 cp, ct, cu;
  cp.x = doc["Actors"]["cpx"].get_float();
  cp.y = doc["Actors"]["cpy"].get_float();
  cp.z = doc["Actors"]["cpz"].get_float();
  ct.x = doc["Actors"]["ctx"].get_float();
  ct.y = doc["Actors"]["cty"].get_float();
  ct.z = doc["Actors"]["ctz"].get_float();
  cu.x = doc["Actors"]["cux"].get_float();
  cu.y = doc["Actors"]["cuy"].get_float();
  cu.z = doc["Actors"]["cuz"].get_float();
  camera::lookat(cp, ct, cu);
  scene_system::m_drawer.push_back(d);
}

} // namespace sinen
