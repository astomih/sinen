#include "texteditor.hpp"
#include <sinen/sinen.hpp>
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <TextEditor.h>

namespace sinen {
TextEditor te;
void texteditor() {
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
  te.SetPalette(p);
  te.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
  te.SetShowWhitespaces(true);
  auto cpos = te.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | fps:%.2f",
              cpos.mLine + 1, cpos.mColumn + 1, te.GetTotalLines(),
              te.IsOverwrite() ? "Ovr" : "Ins", te.CanUndo() ? "*" : " ",
              te.GetLanguageDefinition().mName.c_str(), "aaaa",
              ImGui::GetIO().Framerate);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save", "Ctrl-S", nullptr, te.CanUndo())) {
      }
      if (ImGui::MenuItem("Quit", "Alt-F4"))
        scene::set_state(scene::state::quit);
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

    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Dark palette"))
        te.SetPalette(TextEditor::GetDarkPalette());
      if (ImGui::MenuItem("Light palette"))
        te.SetPalette(TextEditor::GetLightPalette());
      if (ImGui::MenuItem("Retro blue palette"))
        te.SetPalette(TextEditor::GetRetroBluePalette());
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("App")) {
      if (ImGui::MenuItem("Run", "F5", nullptr, true))
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  te.Render("Code");
}
std::string get_text() { return te.GetText(); }
} // namespace sinen