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
  // Table handler
  using table_handler = void *;
  /**
   * @brief Cleate new table object
   *
   * @param table_name table name string
   * @return table_handler table handler
   */
  static table_handler new_table(std::string_view tableName);
  /**
   * @brief Register function
   *
   * @param name function name in Lua
   * @param function function ptr
   * @param table table handler
   */
  static void register_function(std::string_view name,
                                std::function<void()> function,
                                table_handler = nullptr);

private:
};
} // namespace sinen
#endif // !SINEN_SCRIPT_HPP
