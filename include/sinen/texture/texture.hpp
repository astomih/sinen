#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include "../color/color.hpp"
#include "../math/matrix4.hpp"
#include "../utility/handle_t.hpp"
#include "../vertex/vertex.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

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
   * @brief Copy constructor
   *
   * @param other
   */
  Texture(const Texture &other);
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

  Vector2 size();

  Texture &operator=(const Texture &other);

  HandleT handle;
  std::shared_ptr<bool> is_need_update;

private:
  HandleT create();
  void destroy();
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
