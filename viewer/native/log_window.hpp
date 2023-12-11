#ifndef SINEN_LOG_WINDOW_HPP
#define SINEN_LOG_WINDOW_HPP
#include <functional>
#include <imgui.h>
#include <string>
namespace sinen {
struct imguilog {
  struct type {
    ImVec4 color;
    std::string str;
  };
  static std::vector<type> logs;
};
void log_window();
} // namespace sinen
#endif