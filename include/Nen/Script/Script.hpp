#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace nen {
class script_system {
public:
  script_system(class manager &_manager);
  ~script_system();
  script_system(const script_system &) = delete;
  script_system(script_system &&) = default;
  bool initialize();
  void *get_state();
  void do_script(std::string_view fileName);

private:
  class implement;
  std::unique_ptr<implement> impl;
  class manager &m_manager;
};
} // namespace nen
