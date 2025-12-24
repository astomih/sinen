#include <asset/audio/audio_system.hpp>
#include <asset/script/script_system.hpp>
#include <graphics/graphics_system.hpp>
#include <math/random_system.hpp>
#include <physics/physics_system.hpp>
#include <platform/input/input_system.hpp>
#include <platform/window/window_system.hpp>

#include <sinen.hpp>

#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

using namespace sinen;

#include <imgui.h>
#include <imgui_impl_sdl3.h>

enum class State { running, quit };

State mGameState = State::quit;
struct ImGuiLog {
  struct Type {
    ImVec4 color;
    String str;
  };
  static Array<Type> logs;
};
Array<ImGuiLog::Type> ImGuiLog::logs =
    Array<ImGuiLog::Type>(GlobalAllocator::get());
bool initialize(int argc, char *argv[]) {
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
void setup() {
  ScriptSystem::runScene();
  PhysicsSystem::postSetup();
  mGameState = State::running;
}

void processInput() {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    WindowSystem::processInput(event);
    InputSystem::processEvent(event);
    switch (event.type) {
    case SDL_EVENT_QUIT: {
      mGameState = State::quit;
    } break;
    default:
      break;
    }
  }
}

void updateScene() {
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
  if (ScriptSystem::hasToReload())
    mGameState = State::quit;
}

int main(const int argc, char *argv[]) {
  if (!initialize(argc, argv)) {
    Logger::critical("Failed to initialize main system");
    return false;
  }
  if (!WindowSystem::initialize("Sinen")) {
    Logger::critical("Failed to initialize window system");
    return false;
  }
  GraphicsSystem::initialize();
  if (!AudioSystem::initialize()) {
    Logger::critical("Failed to initialize audio system");
    AudioSystem::shutdown();
    return false;
  }
  if (!InputSystem::initialize()) {
    Logger::critical("Failed to initialize input system");
    return false;
  }
  if (!PhysicsSystem::initialize()) {
    Logger::critical("Failed to initialize physics system");
    return false;
  }
  if (!ScriptSystem::initialize()) {
    Logger::critical("Failed to initialize script system");
    return false;
  }
  if (!RandomSystem::initialize()) {
    Logger::critical("Failed to initialize random system");
    return false;
  }
  while (true) {
    if (mGameState != State::quit) {
      WindowSystem::prepareFrame();
      InputSystem::prepareForUpdate();
      processInput();
      InputSystem::update();
      updateScene();
      GraphicsSystem::render();
      continue;
    }
    if (ScriptSystem::hasToReload() && !ScriptSystem::hasToQuit()) {
      setup();
      ScriptSystem::doneReload();
      continue;
    }
    break;
  }
  PhysicsSystem::shutdown();
  InputSystem::shutdown();
  AudioSystem::shutdown();
  RandomSystem::shutdown();
  GraphicsSystem::shutdown();
  WindowSystem::shutdown();
  ScriptSystem::shutdown();
  SDL_Quit();
  return 0;
}
