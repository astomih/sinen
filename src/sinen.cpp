#include "core/event/event_system.hpp"
#include <asset/audio/audio_system.hpp>
#include <asset/script/script_system.hpp>
#include <core/event/event_system.hpp>
#include <graphics/graphics_system.hpp>
#include <math/random_system.hpp>
#include <physics/physics_system.hpp>
#include <platform/input/input_system.hpp>
#include <platform/window/window_system.hpp>

#include <sinen.hpp>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

using namespace sinen;

#include <imgui.h>
#include <imgui_impl_sdl3.h>

struct ImGuiLog {
  struct Type {
    ImVec4 color;
    String str;
  };
  static Array<Type> logs;
};
Array<ImGuiLog::Type> ImGuiLog::logs =
    Array<ImGuiLog::Type>(GlobalAllocator::get());

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  Arguments::argc = argc;
  Arguments ::argv.resize(argc);
  for (int i = 0; i < argc; i++) {
    Arguments::argv[i] = argv[i];
  }

  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_EVENTS;
  if (!SDL_Init(initFlags)) {
    Logger::critical("Failed to initialize SDL");
    return SDL_APP_FAILURE;
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

  if (!WindowSystem::initialize("Sinen")) {
    Logger::critical("Failed to initialize window system");
    return SDL_APP_FAILURE;
  }
  GraphicsSystem::initialize();
  if (!AudioSystem::initialize()) {
    Logger::critical("Failed to initialize audio system");
    AudioSystem::shutdown();
    return SDL_APP_FAILURE;
  }
  if (!InputSystem::initialize()) {
    Logger::critical("Failed to initialize input system");
    return SDL_APP_FAILURE;
  }
  if (!PhysicsSystem::initialize()) {
    Logger::critical("Failed to initialize physics system");
    return SDL_APP_FAILURE;
  }
  if (!ScriptSystem::initialize()) {
    Logger::critical("Failed to initialize script system");
    return SDL_APP_FAILURE;
  }
  if (!RandomSystem::initialize()) {
    Logger::critical("Failed to initialize random system");
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
  if (EventSystem::isQuit())
    return SDL_APP_SUCCESS;
  InputSystem::update();
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
  Time::update();
  ScriptSystem::updateScene();
  PhysicsSystem::update();
  GraphicsSystem::render();
  if (ScriptSystem::hasToReload()) {
    ScriptSystem::runScene();
    PhysicsSystem::postSetup();
    ScriptSystem::doneReload();
  }
  WindowSystem::prepareFrame();
  InputSystem::prepareForUpdate();
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ImGui_ImplSDL3_ProcessEvent(event);
  EventSystem::processEvent(*event);
  WindowSystem::processInput(*event);
  InputSystem::processEvent(*event);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  PhysicsSystem::shutdown();
  InputSystem::shutdown();
  AudioSystem::shutdown();
  RandomSystem::shutdown();
  GraphicsSystem::shutdown();
  WindowSystem::shutdown();
  ScriptSystem::shutdown();
  SDL_Quit();
}
