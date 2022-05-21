#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace nen {
class script {
public:
  script();
  ~script();
  script(const script &) = delete;
  script(script &&) = default;
  void *get_state();
  void do_script(std::string_view fileName);

private:
};
} // namespace nen
