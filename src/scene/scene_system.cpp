#include "scene_system.hpp"
#include "../audio/sound_system.hpp"
#include "../input/input_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../window/window_system.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <camera/camera.hpp>
#include <color/color.hpp>
#include <color/palette.hpp>
#include <io/data_stream.hpp>
#include <io/json.hpp>

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
#include <imgui_impl_sdl3.h>
#include <input/keyboard.hpp>
#include <logger/logger.hpp>
#include <sol/sol.hpp>

namespace sinen {
std::unique_ptr<Scene::implements> scene_system::m_impl =
    std::make_unique<Scene::implements>();
Scene::state scene_system::m_game_state = Scene::state::quit;
bool scene_system::is_run_script = true;
uint32_t scene_system::m_prev_tick = 0;
bool scene_system::is_reset = true;
std::string scene_system::m_scene_name = "main";
bool scene_system::initialize() { return true; }
void scene_system::setup() {
  if (is_run_script) {
    sol::state *lua = (sol::state *)script_system::get_state();
    std::string str =
        DataStream::open_as_string(AssetType::Script, current_name() + ".lua");
    lua->do_string(str.data());
  }
  m_impl->setup();
  m_game_state = Scene::state::running;
  m_prev_tick = SDL_GetTicks();
}

void scene_system::process_input() {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    WindowImpl::process_input(event);
    input_system::process_event(event);
    switch (event.type) {
    case SDL_EVENT_QUIT: {
      m_game_state = Scene::state::quit;
    } break;
    default:
      break;
    }
  }
}

void scene_system::update_scene() {
  {
    if (Keyboard::is_pressed(Keyboard::code::F3)) {
      static auto once = []() {
        Renderer::add_imgui_function([]() {
          ImGui::Begin("Debug");
          ImGui::Text("FPS: %.3f", ImGui::GetIO().Framerate);
          ImGui::Text("DeltaTime: %f", ImGui::GetIO().DeltaTime);
          ImGui::Text("BackendPlatformName: %s",
                      ImGui::GetIO().BackendPlatformName);
          ImGui::Text("BackendRendererName: %s",
                      ImGui::GetIO().BackendRendererName);

          ImGui::End();
        });
        return true;
      }();
      Renderer::toggle_show_imgui();
    }
  }
  // calc delta time
  float delta_time = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  constexpr float MAX_DELTA_TIME = 1.f / 60.f;
  if (delta_time > MAX_DELTA_TIME) {
    delta_time = MAX_DELTA_TIME;
  }
  m_prev_tick = SDL_GetTicks();

  if (is_run_script) {
    sol::state *lua = (sol::state *)script_system::get_state();
    (*lua)["delta_time"] = delta_time;
    (*lua)["update"]();
  }
  m_impl->update(delta_time);
  sound_system::update(delta_time);
}
void scene_system::shutdown() {
  m_impl->terminate();
  m_game_state = Scene::state::quit;
}

void scene_system::change(const std::string &scene_file_name) {
  if (scene_file_name.empty()) {
    Scene::set_state(Scene::state::quit);
    is_reset = false;
  } else {
    is_reset = true;
  }

  scene_system::shutdown();
  m_scene_name = scene_file_name;
}

} // namespace sinen
