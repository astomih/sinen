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
  mTicksCount = SDL_GetTicks();
}

void base_scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // Draw sprites, meshes
  GetRenderer().Draw();
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
  float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
  if (deltaTime > 0.05f) {
    deltaTime = 0.05f;
  }
  mTicksCount = SDL_GetTicks();

  this->Update(deltaTime);
  // All actors update
  for (auto &i : m_actor.data) {
    i.second->UpdateActor(deltaTime);
  }
  // erase dead actors
  for (auto itr = m_actor.data.begin(); itr != m_actor.data.end();) {
    if (itr->second->GetState() == base_actor::state::Dead) {
      itr = m_actor.data.erase(itr);
    } else {
      ++itr;
    }
  }
  m_manager.get_sound_system().Update(deltaTime);
}

void base_scene::Setup() {
  auto &lua = get_script().get_sol_state();
  lua["scene"] = this;
  std::string num = "";

  if (m_manager.get_current_scene_number() < 10)
    num = "0";
  this->get_script().DoScript(
      "scene" + num + std::to_string(m_manager.get_current_scene_number()) +
      ".lua");
  lua["setup"]();
}

void base_scene::UnloadData() { m_actor.clear(); }

void base_scene::Update(float deltaTime) {
  std::string num = "";
  if (m_manager.get_current_scene_number() > 10)
    num = "0";
  this->get_script().DoScript(
      "scene" + num + std::to_string(m_manager.get_current_scene_number()) +
      ".lua");
  get_script().get_sol_state()["update"]();
}

void base_scene::Shutdown() { UnloadData(); }

void base_scene::AddGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer().AddGUI(ui);
}
void base_scene::RemoveGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer().RemoveGUI(ui);
}

renderer &base_scene::GetRenderer() { return m_manager.get_renderer(); }
const input_state &base_scene::GetInput() {
  return m_manager.get_input_system().GetState();
}
sound_system &base_scene::GetSound() { return m_manager.get_sound_system(); }

script_system &base_scene::get_script() {
  return m_manager.get_script_system();
}

manager &base_scene::get_manager() { return m_manager; }

void base_scene::change_scene(std::uint32_t scene_number) {
  m_manager.change_scene(scene_number);
}

} // namespace nen
