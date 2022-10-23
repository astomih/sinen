#pragma once
#include <memory>
#include <string_view>
namespace sinen {
class script_system {
public:
  static bool initialize();
  static void shutdown();
  static void *get_state();
  static void do_script(std::string_view fileName);

private:
  class implement;
  static std::unique_ptr<implement> impl;
};
} // namespace sinen
