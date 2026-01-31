#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include <core/buffer/buffer.hpp>
#include <core/data/string.hpp>
#include <gpu/gpu.hpp>
#include <math/color/color.hpp>
#include <math/vector.hpp>

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

  static constexpr const char *metaTableName() { return "sn.Texture"; }
  String tableString() const;
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
  bool load(StringView fileName);
  bool load(const Buffer &buffer);

  bool loadCubemap(StringView fileName);
  /**
   * @brief Load texture from memory
   *
   * @param buffer buffer
   * @param ID
   * @return true
   * @return false
   */
  bool loadFromMemory(Array<char> &buffer);
  bool loadFromMemory(void *pPixels, uint32_t width, uint32_t height,
                      gpu::TextureFormat format, int channels);
  /**
   * @brief Copy texture from another texture
   *
   * @return texture
   */
  Texture copy();

  void fill(const Color &color);

  Vec2 size();

  Ptr<gpu::Texture> texture;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
