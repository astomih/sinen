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
#include <io/dstream.hpp>

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
uint32_t scene_system::m_prev_tick = 0;
bool scene_system::initialize() {
  sol::state *lua = (sol::state *)script_system::get_state();
  std::string str = dstream::open_as_string(
      asset_type::Script, main::get_current_scene_number() + ".lua");
  lua->do_string(str.data());
  (*lua)["setup"]();
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
  sol::state *lua = (sol::state *)script_system::get_state();
  (*lua)["delta_time"] = deltaTime;
  (*lua)["keyboard"] = input::keyboard;
  (*lua)["mouse"] = input::mouse;
  (*lua)["update"]();

  m_impl->update(deltaTime);
  sound_system::update(deltaTime);
}
void scene_system::shutdown() {
  m_impl->terminate();
  m_game_state = scene::state::quit;
}

} // namespace sinen