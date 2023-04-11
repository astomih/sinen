#include "custom_logger.hpp"
#include "log_window.hpp"

#include <imgui.h>

namespace sinen {
void custom_logger() {
  logger::set_output_function(
      [](logger::priority priority, std::string_view str) {
        ImVec4 color;
        switch (priority) {
        case logger::priority::verbose:
          color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
          break;
        case logger::priority::debug:
          color = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
          break;
        case logger::priority::info:
          color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
          break;
        case logger::priority::error:
          color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
          break;
        case logger::priority::warn:
          color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
          break;
        case logger::priority::critical:
          color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
          break;
        default:
          color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
          break;
        }
        imguilog::logs.push_back({color, std::string(str)});
      });
}

} // namespace sinen