#include <cassert>
#include <chrono>
#include <cstring>
#include <functional>

#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/gpu.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <memory>
#include <platform/io/asset_io.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <SDL3/SDL.h>
#include <stb_image.h>

namespace sinen {
static Ptr<gpu::Texture> createNativeTexture(void *pPixels,
                                             gpu::TextureFormat textureFormat,
                                             uint32_t width, uint32_t height,
                                             int channels);
static void updateNativeTexture(Ptr<gpu::Texture> texture, void *pPixels,
                                int channels);

namespace {
struct AsyncTexture2DState {
  std::future<void> future;
  Array<uint8_t> pixels;
  uint32_t width = 0;
  uint32_t height = 0;
  int channels = 4;
  gpu::TextureFormat format = gpu::TextureFormat::R8G8B8A8_UNORM;
  bool ok = false;
};
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
  this->async = state;

  const String path = fileName.data();
  state->future = globalThreadPool().submit([state, path] {
    auto str = AssetIO::openAsString(path);
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

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture =
          createNativeTexture(state->pixels.data(), state->format, state->width,
                              state->height, state->channels);
      this->pendingWidth = state->width;
      this->pendingHeight = state->height;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
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
  this->async = state;

  const Buffer buf = buffer;
  state->future = globalThreadPool().submit([state, buf] {
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

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture =
          createNativeTexture(state->pixels.data(), state->format, state->width,
                              state->height, state->channels);
      this->pendingWidth = state->width;
      this->pendingHeight = state->height;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
  return true;
}

bool Texture::loadFromMemory(Array<char> &buffer) {
  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  auto state = makePtr<AsyncTexture2DState>();
  this->async = state;

  const String bytes(buffer.data(), buffer.size());
  state->future = globalThreadPool().submit([state, bytes] {
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
    state->pixels.resize(sizeBytes);
    std::memcpy(state->pixels.data(), decoded, sizeBytes);
    stbi_image_free(decoded);

    state->width = static_cast<uint32_t>(width);
    state->height = static_cast<uint32_t>(height);
    state->channels = 4;
    state->format = gpu::TextureFormat::R8G8B8A8_UNORM;
    state->ok = true;
  });

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture =
          createNativeTexture(state->pixels.data(), state->format, state->width,
                              state->height, state->channels);
      this->pendingWidth = state->width;
      this->pendingHeight = state->height;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
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

#include <script/luaapi.hpp>
namespace sinen {
String Texture::tableString() const {
  TablePair p;
  p.emplace_back("isLoaded", texture ? "true" : "false");
  return convert("sn.Texture", p, false);
}
} // namespace sinen
