#ifndef SINEN_FONT_SYSTEM_HPP
#define SINEN_FONT_SYSTEM_HPP
#include <string>
#include <string_view>
#include <unordered_map>

namespace sinen {
class font_system {
public:
  /**
   * @brief Font manager class
   *
   */
  using fonts_t =
      std::unordered_map<std::string, std::unordered_map<int32_t, void *>>;
  /**
   * @brief Initialize font system
   *
   */
  static bool initialize();
  /**
   * @brief Shutdown font system
   *
   */
  static void shutdown();
  /**
   * @brief Load font
   *
   * @param font_info info
   */
  static void *load(std::string_view file_name, int32_t point_size);
  /**
   * @brief Unload font
   *
   * @param font_info info
   */
  static void unload(std::string_view file_name, int32_t point_size);

private:
  static fonts_t m_fonts;
};
} // namespace sinen
#endif // SINEN_FONT_SYSTEM_HPP
