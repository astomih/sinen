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
class texture {
public:
  /**
   * @brief Construct a new texture object
   *
   */
  texture();
  /**
   * @brief Destroy the texture object
   *
   */
  ~texture();
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
  texture copy();

  void fill_color(const color &color);
  void blend_color(const color &color);

  vector2 size();

  handle_t handle;
  std::shared_ptr<bool> is_need_update;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
