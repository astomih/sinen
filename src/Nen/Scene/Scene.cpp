#include "Scene/Scene.hpp"
#include "../event/current_event.hpp"
#include "Actor/Actor.hpp"
#include "Input/KeyCode.hpp"
#include "Utility/Singleton.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>

namespace nen {

void base_scene::Initialize() {
  Setup();
  logger::Info("Scene setup.");
  mTicksCount = SDL_GetTicks();
}

void base_scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // Draw sprites, meshes
  mRenderer->Draw();
  mInputSystem->PrepareForUpdate();
  mInputSystem->Update();
}

void base_scene::ProcessInput() {

  while (SDL_PollEvent(&current_event_handle::current_event)) {
    ImGui_ImplSDL2_ProcessEvent(&current_event_handle::current_event);
    GetRenderer()->GetWindow()->ProcessInput();
    switch (current_event_handle::current_event.type) {
    case SDL_QUIT: {
      mGameState = game_state::Quit;
    } break;
    default:
      break;
    }
  }

  const input_state &state = mInputSystem->GetState();

  if (state.Keyboard.GetKeyState(key_code::ESCAPE) == button_state::Released) {
    mGameState = game_state::Quit;
  }
  if (state.Keyboard.GetKeyState(key_code::F3) == button_state::Pressed) {
    GetRenderer()->toggleShowImGui();
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
  mSoundSystem->Update(deltaTime);
}

void base_scene::Setup() {}

void base_scene::UnloadData() { m_actor.clear(); }

void base_scene::Update(float deltaTime) {}

void base_scene::Shutdown() { UnloadData(); }

void base_scene::AddGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->AddGUI(ui);
}
void base_scene::RemoveGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->RemoveGUI(ui);
}

} // namespace nen
