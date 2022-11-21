#include "log_window.hpp"
#include <imgui.h>
#include <string>
namespace sinen {
std::vector<imguilog::type> imguilog::logs;
void log_window() {
  ImGui::SetNextWindowPos({250, 520});
  ImGui::SetNextWindowSize({780, 200});
  ImGui::Begin("Log", nullptr,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  for (auto &log : imguilog::logs) {
    ImGui::TextColored(log.color, "%s", (log.str).c_str());
  }
  ImGui::End();
}
} // namespace sinen