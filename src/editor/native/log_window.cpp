#include "log_window.hpp"
#include <imgui.h>
#include <string>
namespace sinen {
std::vector<imguilog::type> imguilog::logs;
void log_window() {
  ImGui::Begin("Log");
  for (auto &log : imguilog::logs) {
    ImGui::TextColored(log.color, "%s", (log.str).c_str());
  }
  ImGui::End();
}
} // namespace sinen