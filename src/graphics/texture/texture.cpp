#include <cassert>
#include <cstring>
#include <vector>

#include <core/data/table_string.hpp>
#include <core/logger/log.hpp>
#include <core/profiler.hpp>
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/gpu.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <math/math.hpp>
#include <memory>
#include <platform/io/asset_reader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <SDL3/SDL.h>
#include <ktx.h>
#include <stb_image.h>

namespace sinen {
static Ptr<gpu::Texture> createNativeTexture(void *pPixels,
                                             gpu::TextureFormat textureFormat,
                                             uint32_t width, uint32_t height,
                                             int channels);
struct TextureMipPixels {
  std::vector<uint8_t> pixels;
  uint32_t width = 0;
  uint32_t height = 0;
};
static Ptr<gpu::Texture>
createNativeTexture(const std::vector<TextureMipPixels> &mips,
                    gpu::TextureFormat textureFormat, int channels);
static void updateNativeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                                int channels);

namespace {
constexpr uint8_t ktx2Identifier[] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32,
                                      0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
constexpr uint32_t vkFormatR8Unorm = 9;
constexpr uint32_t vkFormatR8G8Unorm = 16;
constexpr uint32_t vkFormatR8G8B8Unorm = 23;
constexpr uint32_t vkFormatR8G8B8Srgb = 29;
constexpr uint32_t vkFormatR8G8B8A8Unorm = 37;
constexpr uint32_t vkFormatR8G8B8A8Srgb = 43;
constexpr uint32_t vkFormatB8G8R8A8Unorm = 44;
constexpr uint32_t vkFormatB8G8R8A8Srgb = 50;

struct AsyncTexture2DState {
  std::future<void> future;
  String debugName;
  std::vector<uint8_t> pixels;
  std::vector<TextureMipPixels> mips;
  uint32_t width = 0;
  uint32_t height = 0;
  int channels = 4;
  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  bool ok = false;
};

static void scheduleOnPreDraw(std::function<void()> f) {
  Graphics::addPreDrawFunc(std::move(f));
}

static bool hasKtx2Identifier(const void *data, size_t size) {
  return size >= sizeof(ktx2Identifier) &&
         std::memcmp(data, ktx2Identifier, sizeof(ktx2Identifier)) == 0;
}

static uint32_t mipExtent(uint32_t base, uint32_t level) {
  return Math::max(1u, base >> level);
}

static bool decodeKtx2(const uint8_t *bytes, size_t size,
                       AsyncTexture2DState &out) {
  if (!hasKtx2Identifier(bytes, size)) {
    return false;
  }

  ktxTexture2 *texture = nullptr;
  KTX_error_code result = ktxTexture2_CreateFromMemory(
      bytes, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
  if (result != KTX_SUCCESS || !texture) {
    return true;
  }

  auto destroyTexture = [&texture]() {
    if (texture) {
      ktxTexture2_Destroy(texture);
      texture = nullptr;
    }
  };

  if (texture->numDimensions != 2 || texture->numFaces != 1 ||
      texture->numLayers > 1 || texture->baseWidth == 0 ||
      texture->baseHeight == 0) {
    destroyTexture();
    return true;
  }

  bool transcodedToRgba = false;
  if (ktxTexture2_NeedsTranscoding(texture)) {
    result = ktxTexture2_TranscodeBasis(texture, KTX_TTF_RGBA32, 0);
    if (result != KTX_SUCCESS) {
      destroyTexture();
      return true;
    }
    transcodedToRgba = true;
  }

  int srcChannels = 4;
  bool bgra = false;
  if (!transcodedToRgba) {
    switch (texture->vkFormat) {
    case vkFormatR8Unorm:
      srcChannels = 1;
      break;
    case vkFormatR8G8Unorm:
      srcChannels = 2;
      break;
    case vkFormatR8G8B8Unorm:
    case vkFormatR8G8B8Srgb:
      srcChannels = 3;
      break;
    case vkFormatR8G8B8A8Unorm:
    case vkFormatR8G8B8A8Srgb:
      srcChannels = 4;
      break;
    case vkFormatB8G8R8A8Unorm:
    case vkFormatB8G8R8A8Srgb:
      srcChannels = 4;
      bgra = true;
      break;
    default:
      destroyTexture();
      return true;
    }
  }

  out.mips.clear();
  const uint32_t numLevels = Math::max(1u, texture->numLevels);
  auto *data = texture->pData;
  if (!data) {
    destroyTexture();
    return true;
  }
  for (uint32_t level = 0; level < numLevels; ++level) {
    ktx_size_t offset = 0;
    if (ktxTexture2_GetImageOffset(texture, level, 0, 0, &offset) !=
        KTX_SUCCESS) {
      destroyTexture();
      return true;
    }

    const uint32_t width = mipExtent(texture->baseWidth, level);
    const uint32_t height = mipExtent(texture->baseHeight, level);
    const size_t pixelCount = static_cast<size_t>(width) * height;
    TextureMipPixels mip;
    mip.width = width;
    mip.height = height;
    mip.pixels.resize(pixelCount * 4u);

    const uint8_t *src = data + offset;
    for (size_t i = 0; i < pixelCount; ++i) {
      const uint8_t r = src[i * srcChannels + (bgra ? 2 : 0)];
      const uint8_t g = srcChannels > 1 ? src[i * srcChannels + 1] : r;
      const uint8_t b =
          srcChannels > 2 ? src[i * srcChannels + (bgra ? 0 : 2)] : r;
      const uint8_t a = srcChannels > 3 ? src[i * srcChannels + 3] : 255;
      mip.pixels[i * 4 + 0] = r;
      mip.pixels[i * 4 + 1] = g;
      mip.pixels[i * 4 + 2] = b;
      mip.pixels[i * 4 + 3] = a;
    }
    out.mips.push_back(std::move(mip));
  }

  if (!out.mips.empty()) {
    out.pixels = out.mips[0].pixels;
    out.width = out.mips[0].width;
    out.height = out.mips[0].height;
    out.channels = 4;
    out.format = gpu::TextureFormat::R8G8B8A8_UNORM;
    out.ok = true;
  }

  destroyTexture();
  return true;
}
} // namespace
Texture::Texture() { texture = nullptr; }
Texture::Texture(int width, int height) {
  Array<uint8_t> pixels(width * height * 4, 0);
  texture = createNativeTexture(
      pixels.data(), gpu::TextureFormat::R8G8B8A8_UNORM, width, height, 4);
}
Ptr<Texture> Texture::create() { return makePtr<Texture>(); }
Ptr<Texture> Texture::create(int width, int height) {
  return makePtr<Texture>(width, height);
}

Texture::~Texture() {}

bool Texture::load(StringView fileName) {
  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  auto state = makePtr<AsyncTexture2DState>();
  state->debugName = "Texture::load(" + String(fileName) + ")";
  this->async = state;

  const String path = fileName.data();
  state->future = globalThreadPool().submit([state, path] {
    ZoneScopedN("Texture::load decode");
    auto str = AssetReader::readAsString(path);
    if (decodeKtx2(reinterpret_cast<const uint8_t *>(str.data()), str.size(),
                   *state)) {
      return;
    }
    int width = 0, height = 0, bpp = 0;
    unsigned char *decoded = stbi_load_from_memory(
        reinterpret_cast<unsigned char *>(str.data()),
        static_cast<int>(str.size()), &width, &height, &bpp, 4);
    if (!decoded || width <= 0 || height <= 0) {
      if (decoded)
        stbi_image_free(decoded);
      state->ok = false;
      return;
    }

    const size_t sizeBytes =
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    state->pixels.resize(sizeBytes);
    std::memcpy(state->pixels.data(), decoded, sizeBytes);
    stbi_image_free(decoded);

    state->width = static_cast<uint32_t>(width);
    state->height = static_cast<uint32_t>(height);
    state->channels = 4;
    state->format = gpu::TextureFormat::R8G8B8A8_UNORM;
    state->ok = true;
  });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state] {
        if (state->ok) {
          texture = state->mips.empty()
                        ? createNativeTexture(state->pixels.data(),
                                              state->format, state->width,
                                              state->height, state->channels)
                        : createNativeTexture(state->mips, state->format,
                                              state->channels);
          this->pendingWidth = state->width;
          this->pendingHeight = state->height;
        }
        this->loading = false;
        this->async.reset();
      },
      [this] {
        this->loading = false;
        this->async.reset();
      });
  return true;
}
bool Texture::load(const Buffer &buffer) {
  this->texture.reset();
  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  auto state = makePtr<AsyncTexture2DState>();
  state->debugName = "Texture::load(buffer)";
  this->async = state;

  const Buffer buf = buffer;
  state->future = globalThreadPool().submit([state, buf] {
    ZoneScopedN("Texture::load(buffer) decode");
    if (decodeKtx2(reinterpret_cast<const uint8_t *>(buf.data()), buf.size(),
                   *state)) {
      return;
    }
    int width = 0, height = 0, bpp = 0;
    unsigned char *decoded =
        stbi_load_from_memory(reinterpret_cast<unsigned char *>(buf.data()),
                              buf.size(), &width, &height, &bpp, 4);
    if (!decoded || width <= 0 || height <= 0) {
      if (decoded)
        stbi_image_free(decoded);
      state->ok = false;
      return;
    }

    const size_t sizeBytes =
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    state->pixels.resize(sizeBytes);
    std::memcpy(state->pixels.data(), decoded, sizeBytes);
    stbi_image_free(decoded);

    state->width = static_cast<uint32_t>(width);
    state->height = static_cast<uint32_t>(height);
    state->channels = 4;
    state->format = gpu::TextureFormat::R8G8B8A8_UNORM;
    state->ok = true;
  });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state] {
        if (state->ok) {
          texture = state->mips.empty()
                        ? createNativeTexture(state->pixels.data(),
                                              state->format, state->width,
                                              state->height, state->channels)
                        : createNativeTexture(state->mips, state->format,
                                              state->channels);
          this->pendingWidth = state->width;
          this->pendingHeight = state->height;
        }
        this->loading = false;
        this->async.reset();
      },
      [this] {
        this->loading = false;
        this->async.reset();
      });
  return true;
}

bool Texture::loadFromMemory(Array<char> &buffer) {
  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  auto state = makePtr<AsyncTexture2DState>();
  state->debugName = "Texture::loadFromMemory";
  this->async = state;

  const String bytes(buffer.data(), buffer.size());
  state->future = globalThreadPool().submit([state, bytes] {
    const char *stage = "start";
    try {
    ZoneScopedN("Texture::loadFromMemory decode");
    stage = "decode ktx2";
    if (decodeKtx2(reinterpret_cast<const uint8_t *>(bytes.data()),
                   bytes.size(), *state)) {
      return;
    }
    stage = "stbi decode";
    int width = 0, height = 0, bpp = 0;
    unsigned char *decoded = stbi_load_from_memory(
        reinterpret_cast<const unsigned char *>(bytes.data()),
        static_cast<int>(bytes.size()), &width, &height, &bpp, 4);
    if (!decoded || width <= 0 || height <= 0) {
      if (decoded)
        stbi_image_free(decoded);
      state->ok = false;
      return;
    }

    const size_t sizeBytes =
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    stage = "copy decoded pixels";
    state->pixels.resize(sizeBytes);
    std::memcpy(state->pixels.data(), decoded, sizeBytes);
    stbi_image_free(decoded);

    state->width = static_cast<uint32_t>(width);
    state->height = static_cast<uint32_t>(height);
    state->channels = 4;
    state->format = gpu::TextureFormat::R8G8B8A8_UNORM;
    state->ok = true;
    } catch (const std::exception &e) {
      state->ok = false;
      Log::error("Texture::loadFromMemory failed at {}: {}", stage, e.what());
    } catch (...) {
      state->ok = false;
      Log::error("Texture::loadFromMemory failed at {} with an unknown "
                 "exception",
                 stage);
    }
  });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state] {
        if (state->ok) {
          texture = state->mips.empty()
                        ? createNativeTexture(state->pixels.data(),
                                              state->format, state->width,
                                              state->height, state->channels)
                        : createNativeTexture(state->mips, state->format,
                                              state->channels);
          this->pendingWidth = state->width;
          this->pendingHeight = state->height;
        }
        this->loading = false;
        this->async.reset();
      },
      [this] {
        this->loading = false;
        this->async.reset();
      });
  return true;
}

bool Texture::loadFromMemory(void *pPixels, uint32_t width, uint32_t height,
                             gpu::TextureFormat format, int channels) {
  texture = createNativeTexture(pPixels, format, width, height, channels);
  return true;
}

bool Texture::loadPixels(const Buffer &buffer, uint32_t width, uint32_t height,
                         gpu::TextureFormat format, int channels) {
  if (width == 0 || height == 0 || channels <= 0) {
    return false;
  }
  const size_t required = static_cast<size_t>(width) *
                          static_cast<size_t>(height) *
                          static_cast<size_t>(channels);
  if (static_cast<size_t>(buffer.size()) < required) {
    return false;
  }
  texture = createNativeTexture(buffer.data(), format, width, height, channels);
  return texture != nullptr;
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

Ptr<Texture> Texture::copy() {
  auto dst = Texture::create();
  dst->texture = texture; // ?
  return dst;
}

Vec2 Texture::size() {
  if (texture) {
    auto desc = texture->getCreateInfo();
    return Vec2(static_cast<float>(desc.width),
                static_cast<float>(desc.height));
  }
  if (loading && pendingWidth > 0 && pendingHeight > 0) {
    return Vec2(static_cast<float>(pendingWidth),
                static_cast<float>(pendingHeight));
  }
  return Vec2(0.0f, 0.0f);
}

static void writeTextureLevel(Ptr<gpu::Texture> texture, void *pPixels,
                              int channels, uint32_t mipLevel, uint32_t width,
                              uint32_t height) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
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
    dst.mipLevel = mipLevel;
    dst.texture = texture;
    copyPass->uploadTexture(src, dst, true);
    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}

static void writeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                         int channels) {
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;
  writeTextureLevel(texture, pPixels, channels, 0, width, height);
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
    info.debugName = "Texture 2D";
    texture = device->createTexture(info);
  }
  writeTexture(texture, pPixels, channels);
  return texture;
}

Ptr<gpu::Texture> createNativeTexture(const std::vector<TextureMipPixels> &mips,
                                      gpu::TextureFormat textureFormat,
                                      int channels) {
  if (mips.empty()) {
    return nullptr;
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();

  gpu::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.width = mips[0].width;
  info.height = mips[0].height;
  info.layerCountOrDepth = 1;
  info.format = textureFormat;
  info.usage = gpu::TextureUsage::Sampler;
  info.numLevels = static_cast<UInt32>(mips.size());
  info.sampleCount = gpu::SampleCount::x1;
  info.type = gpu::TextureType::Texture2D;
  info.debugName = "Texture 2D mip chain";
  auto texture = device->createTexture(info);
  if (!texture) {
    return nullptr;
  }

  for (uint32_t level = 0; level < mips.size(); ++level) {
    writeTextureLevel(texture, const_cast<uint8_t *>(mips[level].pixels.data()),
                      channels, level, mips[level].width, mips[level].height);
  }
  return texture;
}

void updateNativeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                         int channels) {
  writeTexture(texture, pPixels, channels);
}
} // namespace sinen

namespace sinen {
String Texture::tableString() const {
  TablePair p;
  p.emplace_back("isLoaded", texture ? "true" : "false");
  return convert("sn.Texture", p, false);
}

} // namespace sinen
