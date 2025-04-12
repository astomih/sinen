#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include "../color/color.hpp"
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
  bool load_from_memory(std::vector<char> &buffer);
  /**
   * @brief Copy texture from another texture
   *
   * @return texture
   */
  Texture copy();

  void fill_color(const Color &color);
  void blend_color(const Color &color);

  glm::vec2 size();

  std::shared_ptr<void> textureData;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
