#ifndef SINEN_SCRIPT_HPP
#define SINEN_SCRIPT_HPP
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace sinen {
/**
 * @brief Script class
 *
 */
class script {
public:
  /**
   * @brief Get the state object
   *
   * @return void* sol::state
   */
  static void *get_state();
  /**
   * @brief Do lua script
   *
   * @param fileName lua script file
   */
  static void do_script(std::string_view fileName);

private:
};
} // namespace sinen
#endif // !SINEN_SCRIPT_HPP
