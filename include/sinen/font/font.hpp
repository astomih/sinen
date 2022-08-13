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
  font() : m_font(nullptr), isLoad(false), pointSize(0){};
  font(std::string_view file_name, int32_t point);
  ~font();

  /**
   * @brief rendering quality
   *
   */
  enum class quality {
    /**
     * @brief speedy
     *
     */
    Solid,
    /**
     * @brief slowly, but high quality
     *
     */
    Shaded,
    /**
     * @brief Shaded without backimage
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
  bool isLoaded() { return isLoad; }
  void unload();

  /**
   * @brief render to texture
   *
   * @param text require UTF-8
   * @param _color font color
   * @param _quality font quality
   * @param backgroundColor background color
   * @return std::shared_ptr<texture> rendered texture
   */
  void render_text(texture &tex, std::string_view text,
                   const color &_color = palette::White);

private:
  int pointSize;
  bool isLoad;
  std::string fontName;
  void *m_font;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
