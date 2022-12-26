#include "texteditor.hpp"
#include <sinen/sinen.hpp>
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <TextEditor.h>

namespace sinen {
std::string texteditor::path;
TextEditor te;
void texteditor::display() {
  ImGui::SetNextWindowPos({0, 0});
  static auto once = []() {
    ImGui::SetNextWindowSize({250, 360});
    return true;
  }();
  ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_MenuBar);
  te.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
  te.SetShowWhitespaces(true);
  auto cpos = te.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s ", cpos.mLine + 1,
              cpos.mColumn + 1, te.GetTotalLines(),
              te.IsOverwrite() ? "Ovr" : "Ins", te.CanUndo() ? "*" : " ",
              te.GetLanguageDefinition().mName.c_str());
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        data_stream::write(asset_type::Script, path, te.GetText());
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      bool ro = te.IsReadOnly();
      if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
        te.SetReadOnly(ro);
      ImGui::Separator();

      if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                          !ro && te.CanUndo()))
        te.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && te.CanRedo()))
        te.Redo();

      ImGui::Separator();

      if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, te.HasSelection()))
        te.Copy();
      if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && te.HasSelection()))
        te.Cut();
      if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && te.HasSelection()))
        te.Delete();
      if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                          !ro && ImGui::GetClipboardText() != nullptr))
        te.Paste();

      ImGui::Separator();

      if (ImGui::MenuItem("Select all", nullptr, nullptr))
        te.SetSelection(TextEditor::Coordinates(),
                        TextEditor::Coordinates(te.GetTotalLines(), 0));

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  te.Render("Code");
  ImGui::End();
}
std::string texteditor::get_text() { return te.GetText(); }
void texteditor::set_text(const std::string &text) { te.SetText(text); }
void texteditor::set_script_name(const std::string &path) {
  texteditor::path = path;
}
} // namespace sinen
