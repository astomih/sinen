#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include "../../math/color/color.hpp"
#include "../../math/color/palette.hpp"
#include "../texture/texture.hpp"
#include <memory>
#include <string_view>

namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class Font {
public:
  Font() : m_font(nullptr), m_size(0) {};
  Font(int32_t point, std::string_view file_name);
  ~Font();

  bool Load(int point_size);
  /**
   * @brief font load from filepath
   *
   * @param font_path file path from data/fonts
   * @param point_size font size
   * @return true success to load
   * @return false failed to load
   */
  bool Load(int pointSize, std::string_view path);
  /**
   * @brief return already loaded font
   *
   * @return true loaded
   * @return false not load yet
   */
  bool IsLoaded() { return this->m_font != nullptr; }
  /**
   * @brief Unload the font
   */
  void Unload();
  /**
   * @brief Font size
   *
   * @return int
   */
  int Size() { return m_size; }
  /**
   * @brief resize font
   *
   * @param point_size
   */
  void Resize(int point_size);
  /**
   * @brief render to texture
   *
   * @param text require UTF-8
   * @param _color font color
   */
  void RenderText(Texture &tex, std::string_view text,
                  const Color &_color = Palette::white());

private:
  void *m_font;
  int m_size;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
