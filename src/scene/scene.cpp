#include "scene_system.hpp"

#include <sinen.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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
void scene::load_data(std::string_view data_file_name) {
  rapidjson::Document doc;
  auto str = data_stream::open_as_string(asset_type::Scene, data_file_name);
  doc.Parse(str.data());
  draw3d d;
  texture tex;
  tex.fill_color(palette::white());
  d.position.x = doc["Actors"]["px"].GetFloat();
  d.position.y = doc["Actors"]["py"].GetFloat();
  d.position.z = doc["Actors"]["pz"].GetFloat();
  d.rotation.x = doc["Actors"]["rx"].GetFloat();
  d.rotation.y = doc["Actors"]["ry"].GetFloat();
  d.rotation.z = doc["Actors"]["rz"].GetFloat();
  d.scale.x = doc["Actors"]["sx"].GetFloat();
  d.scale.y = doc["Actors"]["sy"].GetFloat();
  d.scale.z = doc["Actors"]["sz"].GetFloat();
  d.vertex_name = "BOX";
  d.texture_handle = tex;
  vector3 cp, ct, cu;
  cp.x = doc["Actors"]["cpx"].GetFloat();
  cp.y = doc["Actors"]["cpy"].GetFloat();
  cp.z = doc["Actors"]["cpz"].GetFloat();
  ct.x = doc["Actors"]["ctx"].GetFloat();
  ct.y = doc["Actors"]["cty"].GetFloat();
  ct.z = doc["Actors"]["ctz"].GetFloat();
  cu.x = doc["Actors"]["cux"].GetFloat();
  cu.y = doc["Actors"]["cuy"].GetFloat();
  cu.z = doc["Actors"]["cuz"].GetFloat();
  camera::lookat(cp, ct, cu);
  scene_system::m_drawer.push_back(d);
}

} // namespace sinen
