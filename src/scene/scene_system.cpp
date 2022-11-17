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
#include <input/input.hpp>
#include <io/data_stream.hpp>

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
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <input/input.hpp>
#include <sol/sol.hpp>

namespace sinen {
std::unique_ptr<scene::implements> scene_system::m_impl =
    std::make_unique<scene::implements>();
scene::state scene_system::m_game_state = scene::state::running;
bool scene_system::is_run_script = true;
uint32_t scene_system::m_prev_tick = 0;
std::list<draw3d> scene_system::m_drawer;
std::vector<scene_system::ref_actor> scene_system::m_actors;
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
    if ((*itr).get().get_state() == actor::state::active) {
      (*itr).get().update(deltaTime);
      itr++;
    } else if ((*itr).get().get_state() == actor::state::dead) {
      itr = m_actors.erase(itr);
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
  for (auto &i : m_drawer) {
    i.draw();
  }
}
void scene_system::shutdown() {
  m_impl->terminate();
  m_game_state = scene::state::quit;
}

void scene_system::add_actor(actor &_actor) { m_actors.push_back(_actor); }
void scene_system::remove_actor(actor &_actor) {
  std::erase_if(m_actors,
                [&_actor](auto &act) { return &act.get() == &_actor; });
}

} // namespace sinen