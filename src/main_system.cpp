#include "main_system.hpp"
#include "asset/audio/audio_system.hpp"
#include "asset/script/script_system.hpp"
#include "graphics/graphics_system.hpp"
#include "physics/physics_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/window/window_system.hpp"
#include <SDL3/SDL.h>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

#include <functional>
#include <iostream>
#include <string>

#include <core/io/file_system.hpp>
#include <core/logger/logger.hpp>
#include <platform/input/keyboard.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>

namespace sinen {
MainSystem::State MainSystem::m_game_state = State::quit;
bool MainSystem::is_run_script = true;
uint32_t MainSystem::m_prev_tick = 0;
bool MainSystem::is_reset = true;
std::string MainSystem::m_scene_name = "main";
std::string MainSystem::basePath = ".";
float MainSystem::deltaTime = 0.f;
struct ImGuiLog {
  struct Type {
    ImVec4 color;
    std::string str;
  };
  static std::vector<Type> logs;
};
std::vector<ImGuiLog::Type> ImGuiLog::logs;
bool MainSystem::initialize(int argc, char *argv[]) {
  MainSystem::argc = argc;
  MainSystem::argv.resize(argc);
  for (int i = 0; i < argc; i++) {
    MainSystem::argv[i] = argv[i];
  }

  Logger::setOutputFunction([&](Logger::priority p, std::string_view str) {
    std::string newStr;
    ImVec4 color;
    switch (p) {
    case Logger::priority::verbose:
      color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
      newStr = "VERBOSE: " + std::string(str);
      break;
    case Logger::priority::debug:
      color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
      newStr = "DEBUG: " + std::string(str);
      break;
    case Logger::priority::info:
      color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
      newStr = "INFO: " + std::string(str);
      break;
    case Logger::priority::error:
      color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
      newStr = "ERROR: " + std::string(str);
      break;
    case Logger::priority::warn:
      color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
      newStr = "WARN: " + std::string(str);
      break;
    case Logger::priority::critical:
      color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
      newStr = "CRITICAL: " + std::string(str);
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
  if (is_run_script) {
    ScriptSystem::RunScene(GetCurrentName());
    PhysicsSystem::PostSetup();
  }
  m_game_state = State::running;
  m_prev_tick = SDL_GetTicks();
}

void MainSystem::process_input() {

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    WindowSystem::process_input(event);
    InputSystem::process_event(event);
    switch (event.type) {
    case SDL_EVENT_QUIT: {
      m_game_state = State::quit;
    } break;
    default:
      break;
    }
  }
}

void MainSystem::update_scene() {
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
  deltaTime = (SDL_GetTicks() - m_prev_tick) / 1000.0f;
  constexpr float MAX_DELTA_TIME = 1.f / 60.f;
  if (deltaTime > MAX_DELTA_TIME) {
    deltaTime = MAX_DELTA_TIME;
  }
  m_prev_tick = SDL_GetTicks();

  if (is_run_script) {
    ScriptSystem::UpdateScene();
  }
  PhysicsSystem::Update();
}
void MainSystem::shutdown() { m_game_state = State::quit; }

void MainSystem::Change(const std::string &sceneFileName,
                        const std::string &basePath) {
  if (sceneFileName.empty()) {
    set_state(State::quit);
    is_reset = false;
  } else {
    is_reset = true;
  }

  MainSystem::shutdown();
  m_scene_name = sceneFileName;
  MainSystem::basePath = basePath;
}

} // namespace sinen
