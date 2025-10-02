#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include "../../math/color/color.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <glm/vec2.hpp>

namespace sinen {
/**
 * @brief Texture
 *
 */
class Texture {
public:
  /**
   * @brief Construct a new texture object
   *
   */
  Texture();
  /**
   * @brief Destroy the texture object
   *
   */
  ~Texture();
  /**
   * @brief Load texture from file
   *
   * @param fileName file name
   * @return true success
   * @return false failed
   */
  bool load(std::string_view fileName);
  /**
   * @brief Load texture from memory
   *
   * @param buffer buffer
   * @param ID
   * @return true
   * @return false
   */
  bool loadFromMemory(std::vector<char> &buffer) const;
  bool loadFromMemory(void *pPixels, uint32_t width, uint32_t height);
  /**
   * @brief Copy texture from another texture
   *
   * @return texture
   */
  Texture copy();

  void fillColor(const Color &color);
  void blendColor(const Color &color);

  glm::vec2 size();

  std::shared_ptr<void> textureData;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
