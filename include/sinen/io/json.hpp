#ifndef SINEN_JSON_HPP
#define SINEN_JSON_HPP
#include <string_view>

namespace sinen {
/**
 * @brief Json read/write class
 *
 */
class json {
public:
  /**
   * @brief Construct a new json object
   *
   */
  json();
  /**
   * @brief Destroy the json object
   *
   */
  ~json();

  /**
   * @brief Read json file
   *
   * @param path
   * @return true
   * @return false
   */
  bool read(const std::string_view &path);
  /**
   * @brief Write json file
   *
   * @param path
   * @return true
   * @return false
   */
  bool write(const std::string_view &path);

private:
};

} // namespace sinen
#endif