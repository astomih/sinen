#pragma once
#include <functional>
#include <memory>
#include <string>

namespace sol {
class state;
}

namespace nen {
class script_system {
public:
  script_system();
  ~script_system();
  bool initialize();
  class sol::state &get_sol_state();
  void DoScript(std::string_view fileName);

private:
  class implement;
  std::unique_ptr<implement> m_impl;
};
} // namespace nen
