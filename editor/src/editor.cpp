#include "editor.hpp"
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <ImGuizmo.h>

#include "file_dialog.hpp"
#include "markdown.hpp"
#include "texteditor.hpp"
namespace sinen {

class editor::implements {
public:
  bool is_run = false;
  bool is_save = false;
};

editor::editor() : m_impl(std::make_unique<editor::implements>()) {}
editor::~editor() {}
void editor::setup() {
  m_impl->is_run = false;

  renderer::add_imgui_function(texteditor);
}
void imguizmo() {
  ImGuizmo::BeginFrame();
  static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
  if (ImGui::IsKeyPressed(90))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(69))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(82)) // r Key
    mCurrentGizmoOperation = ImGuizmo::SCALE;
  if (ImGui::RadioButton("Translate",
                         mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  ImGui::SameLine();
  if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
    mCurrentGizmoOperation = ImGuizmo::SCALE;
}
void editor::update(float delta_time) {
  if (input::keyboard.get_key_state(key_code::F3) == button_state::Pressed) {
    renderer::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(texteditor);
    }
  }
  if (input::keyboard.get_key_state(key_code::F4) == button_state::Pressed) {
    renderer ::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(markdown);
    }
  }
  if (input::keyboard.get_key_state(key_code::F2) == button_state::Pressed) {
    renderer ::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(imguizmo);
    }
  }
  if (input::keyboard.get_key_state(key_code::F1) == button_state::Pressed) {
    renderer ::toggle_show_imgui();
    if (renderer::is_show_imgui()) {
      renderer::get_imgui_function().clear();
      renderer::add_imgui_function(func_file_dialog);
    }
  }

  if (renderer::is_show_imgui() &&
      input::keyboard.get_key_state(key_code::F5) == button_state::Pressed) {
    m_impl->is_run = true;
  }
  if (m_impl->is_run) {
    m_impl->is_run = false;
  }
  if (renderer::is_show_imgui() &&
      input::keyboard.is_key_down(key_code::LCTRL) &&
      input::keyboard.get_key_state(key_code::S) == button_state::Pressed) {
    m_impl->is_save = true;
  }
  if (m_impl->is_save) {
    auto str = get_text();
    dstream::write(asset_type::Script,
                   main::get_current_scene_number() + ".lua", str);
    std::cout << str << std::endl;
    m_impl->is_save = false;
  }
}
} // namespace sinen