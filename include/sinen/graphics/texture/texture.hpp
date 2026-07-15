#ifndef SINEN_TEXTURE_HPP
#define SINEN_TEXTURE_HPP
#include <core/buffer/buffer.hpp>
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <gpu/gpu.hpp>
#include <math/color/color.hpp>
#include <math/vector.hpp>

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>


namespace sinen {
/**
 * @brief Texture
 *
 */
class Texture {
public:
  static Ptr<Texture> create();
  static Ptr<Texture> create(int width, int height);

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
  bool loadIrradianceCubemap(StringView fileName, uint32_t faceSize = 64,
                             uint32_t sampleCount = 256);
  bool loadPrefilteredCubemap(StringView fileName, uint32_t faceSize = 128,
                              uint32_t mipLevels = 5,
                              uint32_t sampleCount = 128);
  bool loadBRDFLUT(uint32_t size = 256, uint32_t sampleCount = 512);
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
  bool loadPixels(const Buffer &buffer, uint32_t width, uint32_t height,
                  gpu::TextureFormat format, int channels);
  void setPixelData(const void *pPixels, uint32_t width, uint32_t height,
                    gpu::TextureFormat format, int channels);
  void setFloatPixelData(const float *pPixels, uint32_t width, uint32_t height,
                         int channels);
  void setFloatCubemapData(const std::array<std::vector<float>, 6> &faces,
                           uint32_t faceSize);
  Buffer toPngBuffer() const;
  Buffer toExrBuffer(bool saveAsFp16 = false) const;
  /**
   * @brief Copy texture from another texture
   *
   * @return texture
   */
  Ptr<Texture> copy();

  void fill(const Color &color);

  Vec2 size();

  Ptr<gpu::Texture> getRaw() const { return texture; }

  Texture();
  Texture(int, int);

private:
  Ptr<gpu::Texture> texture;
  std::atomic<bool> loading = false;
  uint32_t pendingWidth = 0;
  uint32_t pendingHeight = 0;
  std::vector<uint8_t> pixels;
  uint32_t pixelWidth = 0;
  uint32_t pixelHeight = 0;
  gpu::TextureFormat pixelFormat = gpu::TextureFormat::Invalid;
  int pixelChannels = 0;
  std::vector<float> floatPixels;
  uint32_t floatPixelWidth = 0;
  uint32_t floatPixelHeight = 0;
  int floatPixelChannels = 0;
  Ptr<void> async;
};
} // namespace sinen
#endif // !SINEN_TEXTURE_HPP
