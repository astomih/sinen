#pragma once
#include <functional>
#include <memory>
#include <string>

#include <sol/sol.hpp>

namespace nen {
class script_system {
public:
  script_system(class manager &_manager) : m_manager(_manager) {}
  ~script_system() = default;
  script_system(const script_system &) = delete;
  script_system(script_system &&) = default;
  bool initialize();
  sol::state &get_sol_state() { return lua; }
  void DoScript(std::string_view fileName);

private:
  class manager &m_manager;
  sol::state lua;
};
} // namespace nen
