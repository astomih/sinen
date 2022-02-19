#include "../event/current_event.hpp"
#include "Input/KeyCode.hpp"
#include "Utility/Singleton.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>

namespace nen {
std::uint32_t base_scene::m_default_handle = 0;

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
  mUpdatingActors = true;

  mUpdatingActors = false;
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
  mUpdatingActors = true;
  for (const auto &actor : m_actor_map) {
    actor.second->UpdateActor(deltaTime);
  }
  mUpdatingActors = false;

  // erase dead actors
  for (auto itr = m_actor_map.begin(); itr != m_actor_map.end();) {
    if (itr->second->GetState() == base_actor::state::Dead) {
      itr = m_actor_map.erase(itr);
    } else {
      ++itr;
    }
  }

  // move pending actors
  for (auto &pending : m_pending_actor_map) {
    pending.second->ComputeWorldTransform();
    m_actor_map.emplace(pending.first, std::move(pending.second));
  }
  m_pending_actor_map.clear();

  mSoundSystem->Update(deltaTime);
}

void base_scene::Setup() {}

void base_scene::UnloadData() { m_actor_map.clear(); }

void base_scene::Update(float deltaTime) {}

void base_scene::Shutdown() { UnloadData(); }

void base_scene::AddGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->AddGUI(ui);
}
void base_scene::RemoveGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->RemoveGUI(ui);
}

} // namespace nen
