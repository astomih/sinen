#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include "../../math/color/color.hpp"
#include "../../math/color/palette.hpp"
#include "../texture/texture.hpp"
#include <asset/model/mesh.hpp>

#include <memory>
#include <string_view>
namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class Font {
public:
  Font();
  Font(int32_t point, std::string_view file_name);
  ~Font();

  bool load(int point_size);
  /**
   * @brief font load from asset
   *
   * @param font_path file path from data/fonts
   * @param point_size font size
   * @return true success to load
   * @return false failed to load
   */
  bool load(int pointSize, std::string_view path);
  bool loadFromPath(int pointSize, std::string_view path);
  /**
   * @brief return already loaded font
   *
   * @return true loaded
   * @return false not load yet
   */
  bool isLoaded() { return font != nullptr; }
  /**
   * @brief Unload the font
   */
  void unload();
  /**
   * @brief Font size
   *
   * @return int
   */
  int size() const { return m_size; }
  /**
   * @brief resize font
   *
   * @param point_size
   */
  void resize(int point_size);

  Texture getAtlas() const;
  Mesh getTextMesh(std::string_view text) const;

private:
  struct Wrapper;
  std::unique_ptr<Wrapper> font;
  int m_size;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
