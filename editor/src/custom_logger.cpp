#include "custom_logger.hpp"
#include "log_window.hpp"

#include <imgui.h>

namespace sinen {

void imgui_logger::debug(std::string_view str) {
  imguilog::logs.push_back(
      {ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "SINEN_DEBUG: " + std::string(str)});
}

void imgui_logger::info(std::string_view str) {
  imguilog::logs.push_back(
      {ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SINEN_INFO: " + std::string(str)});
}

void imgui_logger::error(std::string_view str) {
  imguilog::logs.push_back(
      {ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "SINEN_ERROR: " + std::string(str)});
}

void imgui_logger::warn(std::string_view str) {
  imguilog::logs.push_back(
      {ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "SINEN_WARNING: " + std::string(str)});
}

void imgui_logger::fatal(std::string_view str) {
  imguilog::logs.push_back(
      {ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "SINEN_FATAL: " + std::string(str)});
}

} // namespace sinen