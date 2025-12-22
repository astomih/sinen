#include "main_system.hpp"
#include "asset/audio/audio_system.hpp"
#include "asset/script/script_system.hpp"
#include "core/data/string.hpp"
#include "graphics/graphics_system.hpp"
#include "physics/physics_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/window/window_system.hpp"
#include <SDL3/SDL.h>
#include <graphics/graphics.hpp>
#include <memory_resource>
#include <platform/window/window.hpp>

#include <functional>
#include <iostream>
#include <string>

#include <core/allocator/global_allocator.hpp>
#include <core/allocator/tlsf_allocator.hpp>
#include <core/io/file_system.hpp>
#include <core/logger/logger.hpp>
#include <platform/input/keyboard.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>

namespace sinen {
MainSystem::State MainSystem::mGameState = MainSystem::State::quit;
bool MainSystem::isRunScript = true;
uint32_t MainSystem::mPrevTick = 0;
bool MainSystem::isReset = true;
String MainSystem::mSceneName = "main";
String MainSystem::basePath = ".";
float MainSystem::mDeltaTime = 0.f;
struct ImGuiLog {
  struct Type {
    ImVec4 color;
    String str;
  };
  static Array<Type> logs;
};
Array<ImGuiLog::Type> ImGuiLog::logs =
    Array<ImGuiLog::Type>(GlobalAllocator::get());
bool MainSystem::initialize(int argc, char *argv[]) {
  MainSystem::argc = argc;
  MainSystem::argv.resize(argc);
  for (int i = 0; i < argc; i++) {
    MainSystem::argv[i] = argv[i];
  }

  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_EVENTS;
  if (!SDL_Init(initFlags)) {
    Logger::critical("Failed to initialize SDL");
    return false;
  }

  Logger::setOutputFunction([&](Logger::priority p, StringView str) {
    String newStr;
    ImVec4 color;
    switch (p) {
    case Logger::priority::verbose:
      color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
      newStr = "VERBOSE: " + String(str);
      break;
    case Logger::priority::debug:
      color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
      newStr = "DEBUG: " + String(str);
      break;
    case Logger::priority::info:
      color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
      newStr = "INFO: " + String(str);
      break;
    case Logger::priority::error:
      color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
      newStr = "ERROR: " + String(str);
      break;
    case Logger::priority::warn:
      color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
      newStr = "WARN: " + String(str);
      break;
    case Logger::priority::critical:
      color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
      newStr = "CRITICAL: " + String(str);
      break;
    default:
      color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
      break;
    }
    ImGuiLog::logs.push_back({color, newStr});
  });
  return true;
}
void MainSystem::setup() {
  if (isRunScript) {
    ScriptSystem::runScene(getCurrentName());
    PhysicsSystem::postSetup();
  }
  mGameState = State::running;
  mPrevTick = SDL_GetTicks();
}

void MainSystem::processInput() {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    WindowSystem::processInput(event);
    InputSystem::processEvent(event);
    switch (event.type) {
    case SDL_EVENT_QUIT: {
      mGameState = MainSystem::State::quit;
    } break;
    default:
      break;
    }
  }
}

void MainSystem::updateScene() {
  {
    if (Keyboard::isPressed(Keyboard::Code::F3) ||
        KeyInput::isPressed(KeyInput::AC_BACK)) {
      Graphics::toggleShowImGui();
      if (Graphics::isShowImGui()) {
        Graphics::addImGuiFunction([&]() {
          // Log Window
          ImGui::SetNextWindowPos(ImVec2(0, Window::size().y * (3.f / 4.f)),
                                  ImGuiCond_Always);
          ImGui::SetNextWindowSize(
              ImVec2(Window::size().x, Window::size().y * (1.f / 4.f)),
              ImGuiCond_Always);
          ImGui::Begin("Log", nullptr, ImGuiWindowFlags_NoResize);
          for (auto &log : ImGuiLog::logs) {
            ImGui::TextColored(log.color, "%s", (log.str).c_str());
          }
          ImGui::End();
        });
      } else {
        Graphics::getImGuiFunction().clear();
      }
    }
  }
  // calc delta time
  mDeltaTime = (SDL_GetTicks() - mPrevTick) / 1000.0f;
  constexpr float maxDeltaTime = 1.f / 60.f;
  if (mDeltaTime > maxDeltaTime) {
    mDeltaTime = maxDeltaTime;
  }
  mPrevTick = SDL_GetTicks();

  if (isRunScript) {
    ScriptSystem::updateScene();
  }
  PhysicsSystem::update();
}
void MainSystem::shutdown() { mGameState = State::quit; }

void MainSystem::change(StringView sceneFileName, StringView basePath) {
  if (sceneFileName.empty()) {
    setState(State::quit);
    isReset = false;
  } else {
    isReset = true;
  }

  MainSystem::shutdown();
  mSceneName = String(sceneFileName.data());
  MainSystem::basePath = String(basePath.data());
}

} // namespace sinen
