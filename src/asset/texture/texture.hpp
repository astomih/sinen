#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include <graphics/rhi/rhi.hpp>
#include <math/color/color.hpp>

#include <string_view>

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
  Texture(int width, int height);
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
  bool loadFromPath(std::string_view path);
  /**
   * @brief Load texture from memory
   *
   * @param buffer buffer
   * @param ID
   * @return true
   * @return false
   */
  bool loadFromMemory(Array<char> &buffer);
  bool loadFromMemory(void *pPixels, uint32_t width, uint32_t height);
  /**
   * @brief Copy texture from another texture
   *
   * @return texture
   */
  Texture copy();

  void fill(const Color &color);

  glm::vec2 size();

  Ptr<rhi::Texture> texture;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
