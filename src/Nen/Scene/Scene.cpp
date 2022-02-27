#include <Nen.hpp>

#include "../event/current_event.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <SDL.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>

namespace nen {
base_scene::base_scene(manager &_manager) : m_manager(_manager) {}

void base_scene::Initialize() {
  Setup();
  m_prev_tick = SDL_GetTicks();
}

void base_scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // Draw sprites, meshes
  GetRenderer().render();
  m_manager.get_input_system().PrepareForUpdate();
  m_manager.get_input_system().Update();
}

void base_scene::ProcessInput() {

  while (SDL_PollEvent(&current_event_handle::current_event)) {
    ImGui_ImplSDL2_ProcessEvent(&current_event_handle::current_event);
    GetRenderer().GetWindow().ProcessInput();
    switch (current_event_handle::current_event.type) {
    case SDL_QUIT: {
      mGameState = game_state::Quit;
    } break;
    default:
      break;
    }
  }

  const input_state &state = m_manager.get_input_system().GetState();

  if (state.Keyboard.GetKeyState(key_code::ESCAPE) == button_state::Released) {
    mGameState = game_state::Quit;
  }
  if (state.Keyboard.GetKeyState(key_code::F3) == button_state::Pressed) {
    GetRenderer().toggleShowImGui();
  }
  if (mGameState == game_state::Quit)
    return;
}

void base_scene::UpdateScene() {
  // calc delta time
  float deltaTime = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  if (deltaTime > 0.05f) {
    deltaTime = 0.05f;
  }
  m_prev_tick = SDL_GetTicks();

  this->Update(deltaTime);
  m_manager.get_sound_system().Update(deltaTime);
}

void base_scene::Setup() {
  auto &lua = get_script().get_sol_state();
  std::string num = "";

  if (m_manager.get_current_scene_number() < 10)
    num = "0";
  this->get_script().DoScript(
      "scene" + num + std::to_string(m_manager.get_current_scene_number()) +
      ".lua");
  lua["setup"]();
}

void base_scene::UnloadData() {}

void base_scene::Update(float deltaTime) {
  /*
  std::string num = "";
  if (m_manager.get_current_scene_number() > 10)
    num = "0";
  this->get_script().DoScript(
      "scene" + num + std::to_string(m_manager.get_current_scene_number()) +
      ".lua");
  */
  get_script().get_sol_state()["update"]();
  get_script().get_sol_state()["delta_time"] = deltaTime;
}

void base_scene::Shutdown() { UnloadData(); }

renderer &base_scene::GetRenderer() { return m_manager.get_renderer(); }
const input_state &base_scene::GetInput() {
  return m_manager.get_input_system().GetState();
}
sound_system &base_scene::GetSound() { return m_manager.get_sound_system(); }

script_system &base_scene::get_script() {
  return m_manager.get_script_system();
}
texture_system &base_scene::get_texture() {
  return m_manager.get_texture_system();
}

font_system &base_scene::get_font() { return m_manager.get_font_system(); }

void base_scene::change_scene(std::uint32_t scene_number) {
  m_manager.change_scene(scene_number);
}

} // namespace nen
