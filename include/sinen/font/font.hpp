#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include "../color/color.hpp"
#include "../color/palette.hpp"
#include "../texture/texture.hpp"
#include <memory>

namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class font {
public:
  font() : m_font(nullptr), m_size(0){};
  font(std::string_view file_name, int32_t point);
  ~font();
  /**
   * @brief font load from filepath
   *
   * @param font_path file path from data/fonts
   * @param point_size font size
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
  bool is_loaded() { return this->m_font != nullptr; }
  /**
   * @brief Unload the font
   */
  void unload();
  /**
   * @brief Font size
   *
   * @return int
   */
  int size() { return m_size; }
  /**
   * @brief resize font
   *
   * @param point_size
   */
  void resize(int point_size);
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
  int m_size;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
