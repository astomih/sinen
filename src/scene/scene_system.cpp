#include "scene_system.hpp"
#include "../audio/sound_system.hpp"
#include "../event/event_system.hpp"
#include "../input/input_system.hpp"
#include "../main/main_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../window/window_system.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <camera/camera.hpp>
#include <color/color.hpp>
#include <color/palette.hpp>
#include <input/input.hpp>
#include <io/data_stream.hpp>
#include <io/json.hpp>

// For SDL2
#undef main
#include <main/main.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include <camera/camera.hpp>
#include <component/draw3d_component.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <input/input.hpp>
#include <logger/logger.hpp>
#include <sol/sol.hpp>

namespace sinen {
std::unique_ptr<scene::implements> scene_system::m_impl =
    std::make_unique<scene::implements>();
scene::state scene_system::m_game_state = scene::state::running;
bool scene_system::is_run_script = true;
uint32_t scene_system::m_prev_tick = 0;
std::vector<scene_system::actor_ptr> scene_system::m_actors;
bool scene_system::initialize() {
  if (is_run_script) {
    sol::state *lua = (sol::state *)script_system::get_state();
    std::string str = data_stream::open_as_string(
        asset_type::Script, main::get_current_scene_number() + ".lua");
    lua->do_string(str.data());
    (*lua)["setup"]();
  }
  m_impl->setup();
  m_game_state = scene::state::running;
  m_prev_tick = SDL_GetTicks();
  return true;
}

void scene_system::run_loop() {
  process_input();
  update_scene();
  input_system::prepare_for_update();
  input_system::update();
  // Draw sprites, meshes
  render_system::render();
}

void scene_system::process_input() {

  while (SDL_PollEvent(&event_system::current_event)) {
    ImGui_ImplSDL2_ProcessEvent(&event_system::current_event);
    window_system::process_input();
    switch (event_system::current_event.type) {
    case SDL_QUIT: {
      m_game_state = scene::state::quit;
    } break;
    default:
      break;
    }
  }

  if (m_game_state == scene::state::quit)
    return;
}

void scene_system::update_scene() {
  // calc delta time
  float deltaTime = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  if (deltaTime > 0.05f) {
    deltaTime = 0.05f;
  }
  m_prev_tick = SDL_GetTicks();
  for (auto itr = m_actors.begin(); itr != m_actors.end();) {
    if ((*itr)->get_state() == actor::state::active) {
      (*itr)->update(deltaTime);
      itr++;
    } else if ((*itr)->get_state() == actor::state::dead) {
      itr = m_actors.erase(itr);
    } else {
      itr++;
    }
  }
  if (is_run_script) {
    sol::state *lua = (sol::state *)script_system::get_state();
    (*lua)["delta_time"] = deltaTime;
    (*lua)["keyboard"] = input::keyboard;
    (*lua)["mouse"] = input::mouse;
    (*lua)["update"]();
  }
  m_impl->update(deltaTime);
  sound_system::update(deltaTime);
}
void scene_system::shutdown() {
  m_impl->terminate();
  m_game_state = scene::state::quit;
}

void scene_system::add_actor(actor_ptr _actor) { m_actors.push_back(_actor); }

void scene_system::load_data(std::string_view data_file_name) {
  json doc;
  auto str = data_stream::open_as_string(asset_type::Scene, data_file_name);
  doc.parse(str.data());
  {
    vector3 cp, ct, cu;
    cp.x = doc["Camera"]["cpx"].get_float();
    cp.y = doc["Camera"]["cpy"].get_float();
    cp.z = doc["Camera"]["cpz"].get_float();
    ct.x = doc["Camera"]["ctx"].get_float();
    ct.y = doc["Camera"]["cty"].get_float();
    ct.z = doc["Camera"]["ctz"].get_float();
    cu.x = doc["Camera"]["cux"].get_float();
    cu.y = doc["Camera"]["cuy"].get_float();
    cu.z = doc["Camera"]["cuz"].get_float();
    scene::main_camera().lookat(cp, ct, cu);
  }
  texture tex;
  tex.fill_color(palette::white());
  for (int i = 0; i < doc["Actors"].size(); i++) {
    // Actor setting
    auto &act = scene::create_actor();
    auto index = std::string("Actor") + std::to_string(i);
    auto ref = doc["Actors"][index];
    vector3 pos, rotation, scale;
    pos.x = ref["px"].get_float();
    pos.y = ref["py"].get_float();
    pos.z = ref["pz"].get_float();
    act.set_position(pos);
    rotation.x = ref["rx"].get_float();
    rotation.y = ref["ry"].get_float();
    rotation.z = ref["rz"].get_float();
    act.set_rotation(rotation);
    scale.x = ref["sx"].get_float();
    scale.y = ref["sy"].get_float();
    scale.z = ref["sz"].get_float();
    act.set_scale(scale);
    // Component
    auto &d3 = act.create_component<draw3d_component>();
    d3.set_vertex_name("BOX");
    d3.set_texture(tex);
  }
}
} // namespace sinen
