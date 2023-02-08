#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include "../color/color.hpp"
#include "../color/palette.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include <memory>

namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class font {
public:
  font() : m_font(nullptr), is_load(false), point_size(0){};
  font(std::string_view file_name, int32_t point);
  ~font();

  /**
   * @brief rendering quality
   *
   */
  enum class quality {
    /**
     * @brief Speedy, but low quality
     *
     */
    Solid,
    /**
     * @brief Slowly, but high quality
     *
     */
    Shaded,
    /**
     * @brief Shaded without back image(slowly)
     *
     */
    Blended
  };

  /**
   * @brief font load from filepath
   *
   * @param fontName file path
   * @param pointSize font size
   * @return true success to load
   * @return false failed to load
   */
  bool load(std::string_view fontName, int pointSize);
  /**
   * @brief return already loaded font
   *
   * @return true loaded
   * @return false not load yet
   */
  bool is_loaded() { return is_load; }
  /**
   * @brief Unload the font
   */
  void unload();

  /**
   * @brief render to texture
   *
   * @param text require UTF-8
   * @param _color font color
   */
  void render_text(texture &tex, std::string_view text,
                   const color &_color = palette::white());

private:
  void *m_font;
  bool is_load;
  int point_size;
  std::string font_name;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
