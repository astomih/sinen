#ifndef SINEN_SCRIPT_HPP
#define SINEN_SCRIPT_HPP
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace sinen {
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
} // namespace sinen
#endif // !SINEN_SCRIPT_HPP
