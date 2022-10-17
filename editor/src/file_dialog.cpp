#include "file_dialog.hpp"
// imgui
#include <imgui.h>
#include <imgui_impl_sdl.h>
// Added for ImGui
#include <imfilebrowser.h>

namespace sinen {
ImGui::FileBrowser file_dialog;

void func_file_dialog() {
  file_dialog.Open();
  file_dialog.Display();
}

} // namespace sinen
