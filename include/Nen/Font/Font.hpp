#ifndef NEN_FONT_HPP
#define NEN_FONT_HPP
#include "../Math/Vector3.hpp"
#include "../Texture/Texture.hpp"
#include <memory>

namespace nen {
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
  bool LoadFromFile(std::string_view fontName, int pointSize);
  /**
   * @brief return already loaded font
   *
   * @return true loaded
   * @return false not load yet
   */
  bool isLoaded() { return isLoad; }
  void Unload();

  /**
   * @brief render to texture
   *
   * @param text require UTF-8
   * @param _color font color
   * @param _quality font quality
   * @param backgroundColor background color
   * @return std::shared_ptr<texture> rendered texture
   */
  void RenderText(texture &tex, std::string_view text,
                  const color &_color = palette::White,
                  quality _quality = quality::Blended,
                  const color &backgroundColor = palette::Black);

private:
  int pointSize;
  bool isLoad;
  std::string fontName;
  void *m_font;
};
} // namespace nen
#endif