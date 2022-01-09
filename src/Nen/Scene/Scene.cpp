#include "../Render/RendererHandle.hpp"
#include "../event/current_event.hpp"
#include "Input/KeyCode.hpp"
#include "Utility/Singleton.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "src/Nen/event/current_event.hpp"
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>

namespace nen {
base_scene::base_scene() {}

void base_scene::Initialize() {
  mRenderer = renderer_handle::GetRenderer();
  // シーンのデータを読み込み
  Setup();
  logger::Info("Scene setup.");
  // デルタタイムを読み込み
  mTicksCount = SDL_GetTicks();
}

/*
        メインループ
*/
void base_scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // draw sprites, meshes
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
      mGameState = GameState::Quit;
    } break;
    default:
      break;
    }
  }

  const input_state &state = mInputSystem->GetState();

  if (state.Keyboard.GetKeyState(key_code::ESCAPE) == button_state::Released) {
    mGameState = GameState::Quit;
  }
  if (state.Keyboard.GetKeyState(key_code::F3) == button_state::Pressed) {
    GetRenderer()->toggleShowImGui();
  }
  if (mGameState == GameState::Quit)
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
  for (auto actor : mActors) {
    actor->UpdateActor(deltaTime);
  }
  mUpdatingActors = false;

  // erase dead actors
  for (auto it = mActors.begin(); it != mActors.end();) {
    if ((*it)->GetState() == base_actor::state::Dead) {
      it = mActors.erase(it);
    } else {
      ++it;
    }
  }

  // move pending actors
  for (auto pending : mPendingActors) {
    pending->ComputeWorldTransform();
    mActors.emplace_back(pending);
  }
  mPendingActors.clear();

  mSoundSystem->Update(deltaTime);
}

void base_scene::Setup() {}

void base_scene::UnloadData() { mActors.clear(); }

void base_scene::Update(float deltaTime) {}

void base_scene::Shutdown() { UnloadData(); }

void base_scene::AddGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->AddGUI(ui);
}
void base_scene::RemoveGUI(std::shared_ptr<ui_screen> ui) {
  GetRenderer()->RemoveGUI(ui);
}

} // namespace nen
