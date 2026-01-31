#include <SDL3/SDL.h>
#include <asset/texture/texture.hpp>
#include <cassert>
#include <graphics/graphics.hpp>
#include <memory>
#include <platform/io/asset_io.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <gpu/gpu.hpp>

namespace sinen {
static Ptr<gpu::Texture> createNativeTexture(void *pPixels,
                                             gpu::TextureFormat textureFormat,
                                             uint32_t width, uint32_t height,
                                             int channels);
static void updateNativeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                                int channels);
Texture::Texture() { this->texture = nullptr; }
Texture::Texture(int width, int height) {
  Array<uint8_t> pixels(width * height * 4, 0);
  this->texture = createNativeTexture(
      pixels.data(), gpu::TextureFormat::R8G8B8A8_UNORM, width, height, 4);
}
Texture::~Texture() {}

bool Texture::load(StringView fileName) {
  unsigned char *pixels;
  int width;
  int height;
  int bpp;

  auto str = AssetIO::openAsString(fileName);

  pixels = stbi_load_from_memory(reinterpret_cast<unsigned char *>(str.data()),
                                 str.size(), &width, &height, &bpp, 4);

  texture = createNativeTexture(pixels, gpu::TextureFormat::R8G8B8A8_UNORM,
                                width, height, 4);
  return true;
}
bool Texture::load(const Buffer &buffer) {

  int width, height;
  int bpp;

  auto *pixels =
      stbi_load_from_memory(reinterpret_cast<unsigned char *>(buffer.data()),
                            buffer.size(), &width, &height, &bpp, 4);
  texture = createNativeTexture(pixels, gpu::TextureFormat::R8G8B8A8_UNORM,
                                width, height, 4);
  return true;
}

bool Texture::loadFromMemory(Array<char> &buffer) {
  int width;
  int height;
  int bpp;

  auto *pixels =
      stbi_load_from_memory(reinterpret_cast<unsigned char *>(buffer.data()),
                            buffer.size(), &width, &height, &bpp, 4);

  this->texture = createNativeTexture(
      pixels, gpu::TextureFormat::R8G8B8A8_UNORM, width, height, 4);
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height,
                             gpu::TextureFormat format, int channels) {
  texture = createNativeTexture(pPixels, format, width, height, channels);
  return true;
}

void Texture::fill(const Color &color) {
  if (texture) {

    auto w = texture->getCreateInfo().width;
    auto h = texture->getCreateInfo().height;
    Array<uint8_t> pixels(w * h * 4, 0);
    for (int i = 0; i < w * h * 4; i += 4) {
      pixels[i + 0] = color.r * 255;
      pixels[i + 1] = color.g * 255;
      pixels[i + 2] = color.b * 255;
      pixels[i + 3] = color.a * 255;
    }
    updateNativeTexture(texture, pixels.data(), 4);
  } else {
    Array<uint8_t> pixels(4, 0);
    pixels[0] = color.r * 255;
    pixels[1] = color.g * 255;
    pixels[2] = color.b * 255;
    pixels[3] = color.a * 255;
    texture = createNativeTexture(pixels.data(),
                                  gpu::TextureFormat::R8G8B8A8_UNORM, 1, 1, 4);
  }
}

Texture Texture::copy() {
  Texture dst;
  dst.texture = texture; // ?
  return dst;
}

Vec2 Texture::size() {
  auto desc = texture->getCreateInfo();
  return Vec2(static_cast<float>(desc.width), static_cast<float>(desc.height));
}

static void writeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                         int channels) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;
  Ptr<gpu::TransferBuffer> transferBuffer;
  {
    gpu::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * channels;
    info.usage = gpu::TransferBufferUsage::Upload;
    transferBuffer = device->createTransferBuffer(info);
    auto *pMapped = transferBuffer->map(true);
    memcpy(pMapped, pPixels, info.size);
    transferBuffer->unmap();
  }
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->acquireCommandBuffer(info);
    auto copyPass = commandBuffer->beginCopyPass();
    gpu::TextureTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    gpu::TextureRegion dst{};
    dst.x = 0;
    dst.y = 0;
    dst.width = width;
    dst.height = height;
    dst.depth = 1;
    dst.texture = texture;
    copyPass->uploadTexture(src, dst, true);
    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}
Ptr<gpu::Texture> createNativeTexture(void *pPixels,
                                      gpu::TextureFormat textureFormat,
                                      uint32_t width, uint32_t height,
                                      int channels) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();

  Ptr<gpu::Texture> texture;
  {
    gpu::Texture::CreateInfo info{};
    info.allocator = allocator;
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 1;
    info.format = textureFormat;
    info.usage = gpu::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = gpu::SampleCount::x1;
    info.type = gpu::TextureType::Texture2D;
    texture = device->createTexture(info);
  }
  writeTexture(texture, pPixels, channels);
  return texture;
}
void updateNativeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                         int channels) {
  writeTexture(texture, pPixels, channels);
}
} // namespace sinen

#include <asset/script/luaapi.hpp>
namespace sinen {
String Texture::tableString() const {
  TablePair p;
  p.emplace_back("isLoaded", texture ? "true" : "false");
  return convert("sn.Texture", p, false);
}
} // namespace sinen
