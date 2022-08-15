#pragma once
#include <memory>
namespace sinen {
class script_system {
public:
  script_system();
  script_system(script_system &s);
  ~script_system();
  bool initialize();
  void shutdown();
  void *get_state();
  void do_script(std::string_view fileName);

private:
  class implement;
  std::unique_ptr<implement> impl;
};

} // namespace sinen
