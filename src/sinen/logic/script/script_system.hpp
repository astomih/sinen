#ifndef SINEN_SCRIPT_SYSTEM_HPP
#define SINEN_SCRIPT_SYSTEM_HPP
#include "pybind11/internal/object.h"
#include <functional>
#include <memory>
#include <string_view>

#include <pybind11/pybind11.h>
namespace sinen {
class script_system {
public:
  static bool initialize();
  static void shutdown();
  static void *get_state();
  static void *get_sol_state();
  static void do_script(std::string_view fileName);
  static void *new_table(std::string_view table_name);
  static void register_function(std::string_view name,
                                std::function<void()> function, void *table);

private:
  class implement;
  static std::unique_ptr<implement> impl;
};
} // namespace sinen

#endif // !SINEN_SCRIPT_SYSTEM_HPP