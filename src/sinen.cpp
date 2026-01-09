#include <asset/audio/audio.hpp>
#include <asset/script/script.hpp>
#include <core/event/event.hpp>
#include <graphics/graphics.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/input.hpp>
#include <platform/io/arguments.hpp>
#include <platform/window/window.hpp>

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

  if (!Window::initialize("Sinen")) {
    Logger::critical("Failed to initialize window");
    return SDL_APP_FAILURE;
  }
  if (!Graphics::initialize()) {
    Logger::critical("Failed to initialize graphics");
    return SDL_APP_FAILURE;
  }
  if (!Audio::initialize()) {
    Logger::critical("Failed to initialize audio");
    Audio::shutdown();
    return SDL_APP_FAILURE;
  }
  if (!Input::initialize()) {
    Logger::critical("Failed to initialize input");
    return SDL_APP_FAILURE;
  }
  if (!Physics::initialize()) {
    Logger::critical("Failed to initialize physics");
    return SDL_APP_FAILURE;
  }
  if (!Script::initialize()) {
    Logger::critical("Failed to initialize script");
    return SDL_APP_FAILURE;
  }
  if (!Random::initialize()) {
    Logger::critical("Failed to initialize random");
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
  if (Event::isQuit())
    return SDL_APP_SUCCESS;
  Input::update();
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
  Script::updateScene();
  Physics::update();
  Graphics::render();
  if (Script::hasToReload()) {
    Script::runScene();
    Physics::postSetup();
    Script::doneReload();
  }
  if (Keyboard::isPressed(Keyboard::Code::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    Window::setFullscreen(fullscreen);
  }
  Window::prepareFrame();
  Input::prepareForUpdate();
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ImGui_ImplSDL3_ProcessEvent(event);
  Event::processEvent(*event);
  Window::processEvent(*event);
  Input::processEvent(*event);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  Physics::shutdown();
  Input::shutdown();
  Audio::shutdown();
  Random::shutdown();
  Graphics::shutdown();
  Window::shutdown();
  Script::shutdown();
  SDL_Quit();
}
