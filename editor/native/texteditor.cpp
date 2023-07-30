#include "texteditor.hpp"
#include <sinen/sinen.hpp>
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <TextEditor.h>

namespace sinen {
std::string texteditor::path;
class texteditor::impl {
public:
  TextEditor te;
};
std::unique_ptr<texteditor::impl> texteditor::pimpl =
    std::make_unique<texteditor::impl>();
void texteditor::display() {
  ImGui::Begin("Script Editor", nullptr, ImGuiWindowFlags_MenuBar);
  pimpl->te.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
  pimpl->te.SetShowWhitespaces(true);
  // Editor Settings
  static char path[256] = {};
  ImGui::InputText("file", path, 256);
  ImGui::SameLine();
  if (ImGui::Button("Open")) {
    auto text = data_stream::open_as_string(asset_type::Script, path);
    pimpl->te.SetText(text);
  }
  auto cpos = pimpl->te.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s ", cpos.mLine + 1,
              cpos.mColumn + 1, pimpl->te.GetTotalLines(),
              pimpl->te.IsOverwrite() ? "Ovr" : "Ins",
              pimpl->te.CanUndo() ? "*" : " ",
              pimpl->te.GetLanguageDefinition().mName.c_str());
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        data_stream::write(asset_type::Script, path, pimpl->te.GetText());
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      bool ro = pimpl->te.IsReadOnly();
      if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
        pimpl->te.SetReadOnly(ro);
      ImGui::Separator();

      if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                          !ro && pimpl->te.CanUndo()))
        pimpl->te.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr,
                          !ro && pimpl->te.CanRedo()))
        pimpl->te.Redo();

      ImGui::Separator();

      if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, pimpl->te.HasSelection()))
        pimpl->te.Copy();
      if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                          !ro && pimpl->te.HasSelection()))
        pimpl->te.Cut();
      if (ImGui::MenuItem("Delete", "Del", nullptr,
                          !ro && pimpl->te.HasSelection()))
        pimpl->te.Delete();
      if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                          !ro && ImGui::GetClipboardText() != nullptr))
        pimpl->te.Paste();

      ImGui::Separator();

      if (ImGui::MenuItem("Select all", nullptr, nullptr))
        pimpl->te.SetSelection(
            TextEditor::Coordinates(),
            TextEditor::Coordinates(pimpl->te.GetTotalLines(), 0));

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  pimpl->te.Render("Code");
  ImGui::End();
}
std::string texteditor::get_text() { return pimpl->te.GetText(); }
void texteditor::set_text(const std::string &text) { pimpl->te.SetText(text); }
void texteditor::set_script_name(const std::string &path) {
  texteditor::path = path;
}
} // namespace sinen
