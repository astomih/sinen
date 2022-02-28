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

#include <sol/sol.hpp>

namespace nen {
scene::scene(manager &_manager) : m_manager(_manager) {}

void scene::Initialize() {
  Setup();
  m_prev_tick = SDL_GetTicks();
}

void scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // Draw sprites, meshes
  GetRenderer().render();
  m_manager.get_input_system().PrepareForUpdate();
  m_manager.get_input_system().Update();
}

void scene::ProcessInput() {

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

void scene::UpdateScene() {
  // calc delta time
  float deltaTime = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  if (deltaTime > 0.05f) {
    deltaTime = 0.05f;
  }
  m_prev_tick = SDL_GetTicks();

  this->Update(deltaTime);
  m_manager.get_sound_system().Update(deltaTime);
}

void scene::Setup() {
  sol::state *lua = (sol::state *)get_script().get_state();
  get_script().DoScript("main.lua");
  (*lua)["setup"]();
}

void scene::UnloadData() {}

void scene::Update(float deltaTime) {
  sol::state *lua = (sol::state *)get_script().get_state();
  (*lua)["delta_time"] = deltaTime;
  (*lua)["update"]();
}

void scene::Shutdown() { UnloadData(); }

renderer &scene::GetRenderer() { return m_manager.get_renderer(); }
const input_state &scene::GetInput() {
  return m_manager.get_input_system().GetState();
}
sound_system &scene::GetSound() { return m_manager.get_sound_system(); }

script_system &scene::get_script() { return m_manager.get_script_system(); }
texture_system &scene::get_texture() { return m_manager.get_texture_system(); }

font_system &scene::get_font() { return m_manager.get_font_system(); }

void scene::change_scene(std::uint32_t scene_number) {
  m_manager.change_scene(scene_number);
}

} // namespace nen
