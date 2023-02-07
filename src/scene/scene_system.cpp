#include "scene_system.hpp"
#include "../audio/sound_system.hpp"
#include "../event/event_system.hpp"
#include "../input/input_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../window/window_system.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <camera/camera.hpp>
#include <color/color.hpp>
#include <color/palette.hpp>
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
#include <component/move_component.hpp>
#include <component/rigidbody_component.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <input/keyboard.hpp>
#include <logger/logger.hpp>
#include <sol/sol.hpp>

namespace sinen {
std::unique_ptr<scene::implements> scene_system::m_impl =
    std::make_unique<scene::implements>();
scene::state scene_system::m_game_state = scene::state::quit;
bool scene_system::is_run_script = true;
uint32_t scene_system::m_prev_tick = 0;
std::vector<scene_system::actor_ptr> scene_system::m_actors;
bool scene_system::initialize() {
  scene::get_component_factory().register_component<draw3d_component>("draw3d");

  scene::get_component_factory().register_component<move_component>("move");

  scene::get_component_factory().register_component<rigidbody_component>(
      "rigidbody");
  return true;
}
void scene_system::setup() {
  if (is_run_script) {
    sol::state *lua = (sol::state *)script_system::get_state();
    std::string str = data_stream::open_as_string(
        asset_type::Script, main::get_current_scene_number() + ".lua");
    lua->do_string(str.data());
  }
  m_impl->setup();
  m_game_state = scene::state::running;
  m_prev_tick = SDL_GetTicks();
}

void scene_system::run_loop() {
  process_input();
  update_scene();
  input_system::prepare_for_update();
  input_system::update();
  render_system::render();
}

void scene_system::process_input() {

  while (SDL_PollEvent(&event_system::current_event)) {
    ImGui_ImplSDL2_ProcessEvent(&event_system::current_event);
    window_system::process_input();
    input_system::process_event();
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
  {
    if (keyboard::is_pressed(keyboard::code::F3)) {
      static auto once = []() {
        renderer::add_imgui_function([]() {
          ImGui::Begin("Debug");
          ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
          ImGui::End();
        });
        return true;
      }();
      renderer::toggle_show_imgui();
    }
  }
  // calc delta time
  float deltaTime = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  constexpr float MAX_DELTA_TIME = 1.f / 60.f;
  if (deltaTime > MAX_DELTA_TIME) {
    deltaTime = MAX_DELTA_TIME;
  }
  m_prev_tick = SDL_GetTicks();
  for (auto itr = m_actors.begin(); itr != m_actors.end();) {
    if ((*itr)->get_state() == actor::state::active) {
      sol::state *lua = (sol::state *)script_system::get_state();
      auto r = lua->require_script(
                      (*itr)->get_script_name(),
                      data_stream::open_as_string(asset_type::Script,
                                                  (*itr)->get_script_name()))
                   .as<sol::table>();
      r["update"]();
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
actor &scene_system::get_actor(const std::string &str) {
  for (auto actor : m_actors) {
    if (actor->get_name() == str) {
      return *actor;
    }
  }
  static actor null_actor;
  null_actor.set_name("null");
  return null_actor;
}

void scene_system::load_data(std::string_view data_file_name) {
  json doc;
  auto str = data_stream::open_as_string(asset_type::Scene, data_file_name);
  doc.parse(str.data());
  {
    vector3 cp, ct, cu;
    auto camera_data = doc["Camera"];
    cp.x = camera_data["Position"]["x"].get_float();
    cp.y = camera_data["Position"]["y"].get_float();
    cp.z = camera_data["Position"]["z"].get_float();
    ct.x = camera_data["Target"]["x"].get_float();
    ct.y = camera_data["Target"]["y"].get_float();
    ct.z = camera_data["Target"]["z"].get_float();
    cu.x = camera_data["Up"]["x"].get_float();
    cu.y = camera_data["Up"]["y"].get_float();
    cu.z = camera_data["Up"]["z"].get_float();
    scene::main_camera().lookat(cp, ct, cu);
  }
  texture tex;
  tex.fill_color(palette::white());
  for (int i = 0; i < doc["Actors"].get_array().size(); i++) {
    // Actor setting
    auto &act = scene::create_actor();
    auto ref = doc["Actors"].get_array()[i];
    act.set_name(ref["Name"].get_string());
    act.set_script_name(ref["Script"].get_string());
    vector3 pos, rotation, scale;
    pos.x = ref["Position"]["x"].get_float();
    pos.y = ref["Position"]["y"].get_float();
    pos.z = ref["Position"]["z"].get_float();
    act.set_position(pos);
    rotation.x = ref["Rotation"]["x"].get_float();
    rotation.y = ref["Rotation"]["y"].get_float();
    rotation.z = ref["Rotation"]["z"].get_float();
    act.set_rotation(rotation);
    scale.x = ref["Scale"]["x"].get_float();
    scale.y = ref["Scale"]["y"].get_float();
    scale.z = ref["Scale"]["z"].get_float();
    act.set_scale(scale);
    for (int j = 0; j < ref["Components"].get_array().size(); j++) {
      auto comp = ref["Components"].get_array()[j];
      auto comp_name = comp.get_string();
      auto c = scene::get_component_factory().create(comp_name, act);
      act.add_component(c);
    }
  }
}
} // namespace sinen
