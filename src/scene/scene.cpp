#include "../event/current_event.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <input/input.hpp>
#include <io/dstream.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

#include "../audio/sound_system.hpp"
#include "../input/input_system.hpp"
#include "../manager/get_system.hpp"
#include "../math/random_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../window/window_system.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <utility/launcher.hpp>

#include <TextEditor.h>
#include <camera/camera.hpp>
#include <imgui.h>
#include <input/input.hpp>
#include <sol/sol.hpp>

namespace nen {
scene::scene() {}

void scene::initialize() {
  Setup();
  m_prev_tick = SDL_GetTicks();
}

void scene::RunLoop() {
  ProcessInput();
  UpdateScene();
  // Draw sprites, meshes
  get_renderer().render();
  get_input().prepare_for_update();
  get_input().update();
}

void scene::ProcessInput() {

  while (SDL_PollEvent(&current_event_handle::current_event)) {
    ImGui_ImplSDL2_ProcessEvent(&current_event_handle::current_event);
    get_window().ProcessInput();
    switch (current_event_handle::current_event.type) {
    case SDL_QUIT: {
      mGameState = game_state::Quit;
    } break;
    default:
      break;
    }
  }

  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    get_renderer().toggle_show_imgui();
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
  get_sound().update(deltaTime);
}

bool is_run = false;
bool is_save = false;
TextEditor editor;
void scene::Setup() {
  const static TextEditor::Palette p = {{
      0xff7f7f7f, // Default
      0xffd69c56, // Keyword
      0xff00ff00, // Number
      0xff7070e0, // String
      0xff70a0e0, // Char literal
      0xffffffff, // Punctuation
      0xff408080, // Preprocessor
      0xffaaaaaa, // Identifier
      0xff9bc64d, // Known identifier
      0xffc040a0, // Preproc identifier
      0xff206020, // Comment (single line)
      0xff406020, // Comment (multi line)
      0x00101010, // Background
      0xffe0e0e0, // Cursor
      0x80a06020, // Selection
      0x800020ff, // ErrorMarker
      0x40f08000, // Breakpoint
      0xff707000, // Line number
      0x40000000, // Current line fill
      0x40808080, // Current line fill (inactive)
      0x40a0a0a0, // Current line edge
  }};
  sol::state *lua = (sol::state *)get_script().get_state();
  std::string str = dstream::open_as_string(
      asset_type::Script, get_current_scene_number() + ".lua");
  lua->do_string(str.data());
  (*lua)["setup"]();
  editor.SetPalette(p);
  editor.SetText(str);
  editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
  editor.SetShowWhitespaces(true);
  get_renderer().add_imgui_function([&]() {
    auto cpos = editor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | fps:%.2f",
                cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                editor.IsOverwrite() ? "Ovr" : "Ins",
                editor.CanUndo() ? "*" : " ",
                editor.GetLanguageDefinition().mName.c_str(),
                std::string(get_current_scene_number() + ".lua").c_str(),
                ImGui::GetIO().Framerate);
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save", "Ctrl-S", nullptr, editor.CanUndo())) {
          is_save = true;
        }
        if (ImGui::MenuItem("Quit", "Alt-F4"))
          this->Quit();
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit")) {
        bool ro = editor.IsReadOnly();
        if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
          editor.SetReadOnly(ro);
        ImGui::Separator();

        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                            !ro && editor.CanUndo()))
          editor.Undo();
        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
          editor.Redo();

        ImGui::Separator();

        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
          editor.Copy();
        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                            !ro && editor.HasSelection()))
          editor.Cut();
        if (ImGui::MenuItem("Delete", "Del", nullptr,
                            !ro && editor.HasSelection()))
          editor.Delete();
        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                            !ro && ImGui::GetClipboardText() != nullptr))
          editor.Paste();

        ImGui::Separator();

        if (ImGui::MenuItem("Select all", nullptr, nullptr))
          editor.SetSelection(
              TextEditor::Coordinates(),
              TextEditor::Coordinates(editor.GetTotalLines(), 0));

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Dark palette"))
          editor.SetPalette(TextEditor::GetDarkPalette());
        if (ImGui::MenuItem("Light palette"))
          editor.SetPalette(TextEditor::GetLightPalette());
        if (ImGui::MenuItem("Retro blue palette"))
          editor.SetPalette(TextEditor::GetRetroBluePalette());
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("App")) {
        if (ImGui::MenuItem("Run", "F5", nullptr, true))
          is_run = true;
#if !defined(MOBILE) && !defined(EMSCRIPTEN)
        if (ImGui::MenuItem("Toggle API")) {
          std::ofstream ofs("api");
          if (get_renderer().get_graphics_api() == graphics_api::OpenGL)
            ofs << "Vulkan";
          else
            ofs << "OpenGL";
        }
#endif
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }
    editor.Render("Code");
  });
}

void scene::UnloadData() { get_renderer().get_imgui_function().clear(); }

void scene::Update(float deltaTime) {
  sol::state *lua = (sol::state *)get_script().get_state();
  (*lua)["delta_time"] = deltaTime;
  (*lua)["keyboard"] = input::keyboard;
  (*lua)["mouse"] = input::mouse;
  (*lua)["camera"] = &get_camera();
  random rand;
  (*lua)["random"] = &rand;
  if (get_renderer().is_show_imgui() &&
      input::keyboard.get_key_state(key_code::F5) == button_state::Pressed) {
    is_run = true;
  }
  if (is_run) {
    auto str = editor.GetText();
    lua->do_string(str);
    (*lua)["setup"]();
    is_run = false;
  }
  if (get_renderer().is_show_imgui() &&
      input::keyboard.is_key_down(key_code::LCTRL) &&
      input::keyboard.get_key_state(key_code::S) == button_state::Pressed) {
    is_save = true;
  }
  if (is_save) {
    auto str = editor.GetText();
    dstream::write(asset_type::Script, get_current_scene_number() + ".lua",
                   str);
    std::cout << str << std::endl;
    is_save = false;
  }
  (*lua)["update"]();
}

void scene::Shutdown() { UnloadData(); }

void scene::change_scene(std::string scene_name) { change_scene(scene_name); }

} // namespace nen
