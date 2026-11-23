#define NOMINMAX
#include "gpu/backend/d3d12/d3d12_device.hpp"
#include "graphics/text_model_cache.hpp"
#include "platform/window/window_native.hpp"

#include <core/allocator/engine_memory.hpp>
#include <graphics/font/font.hpp>
#include <graphics/graphics.hpp>
#include <graphics/render_pass.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL.h>
#include <d3d12sdklayers.h>
#include <gtest/gtest.h>
#include <stb_image.h>

#include <string>

using namespace sinen;
namespace {
class TextCacheTests : public testing::Test {
protected:
  void SetUp() override {
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
    ASSERT_TRUE(Window::initialize("Sinen text cache test"));
    SDL_HideWindow(static_cast<SDL_Window *>(WindowNative::getWindow()));
    ASSERT_TRUE(Graphics::initialize(GPUBackendAPI::D3D12));
    device = Graphics::getDevice();
    std::static_pointer_cast<gpu::d3d12::Device>(device)
        ->getNative()
        ->QueryInterface(IID_PPV_ARGS(&messages));
    if (messages) {
      messages->ClearStoredMessages();
    }
  }
  void TearDown() override {
    if (device) {
      device->waitForGpuIdle();
    }
    Graphics::shutdown();
    if (messages) {
      for (UINT64 i = 0; i < messages->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0;
        messages->GetMessage(i, nullptr, &size);
        std::vector<char> data(size);
        auto *message = reinterpret_cast<D3D12_MESSAGE *>(data.data());
        if (SUCCEEDED(messages->GetMessage(i, message, &size)) &&
            message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR) {
          ADD_FAILURE() << message->pDescription;
        }
      }
    }
    device.reset();
    messages.Reset();
    Window::shutdown();
    SDL_Quit();
  }
  Ptr<gpu::Device> device;
  gpu::d3d12::ComPtr<ID3D12InfoQueue> messages;
};
} // namespace

TEST_F(TextCacheTests, RepeatedTextReusesGpuBuffersWithoutBuildingLods) {
  auto font = Font::create();
  ASSERT_TRUE(font->load(16, FontMethod::Bitmap));
  const auto text = font->makeTextDrawData("Cache: 0123");
  ASSERT_TRUE(text.valid);
  const auto again = font->makeTextDrawData("Cache: 0123");
  EXPECT_EQ(text.mesh.data(), again.mesh.data());
  EXPECT_EQ(text.texture, again.texture);
  TextModelCache cache;
  auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
  auto first = cache.acquire(text.mesh, commands);
  auto promoted = cache.acquire(text.mesh, commands);
  ASSERT_NE(promoted->vertexBuffer, nullptr);
  EXPECT_TRUE(promoted->getMesh().data()->lods.empty());
  auto hit = cache.acquire(again.mesh, commands);
  EXPECT_EQ(promoted, hit);
  EXPECT_EQ(promoted->vertexBuffer, hit->vertexBuffer);
  EXPECT_EQ(promoted->indexBuffer, hit->indexBuffer);
  device->submitCommandBuffer(commands);
}

TEST_F(TextCacheTests, FontCacheEvictsLeastRecentTextAndInvalidatesOnReload) {
  auto font = Font::create();
  ASSERT_TRUE(font->load(8, FontMethod::Bitmap));
  auto hot = font->makeTextDrawData("hot");
  const Ref<Mesh::Data> hotMesh = hot.mesh.data();
  hot = {};
  auto cold = font->makeTextDrawData("cold");
  const Ref<Mesh::Data> coldMesh = cold.mesh.data();
  cold = {};
  for (int i = 0; i < 126; ++i) {
    ASSERT_TRUE(font->makeTextDrawData("text-" + std::to_string(i)).valid);
  }
  ASSERT_FALSE(hotMesh.expired());
  ASSERT_FALSE(coldMesh.expired());
  font->makeTextDrawData("hot");
  font->makeTextDrawData("overflow");
  EXPECT_FALSE(hotMesh.expired());
  EXPECT_TRUE(coldMesh.expired());
  const auto before = font->makeTextDrawData("hot");
  font->resize(20);
  const auto after = font->makeTextDrawData("hot");
  ASSERT_TRUE(after.valid);
  EXPECT_NE(before.mesh.data(), after.mesh.data());
  EXPECT_NE(before.texture, after.texture);
  font->unload();
  EXPECT_FALSE(font->isLoaded());
  EXPECT_FALSE(font->makeTextDrawData("hot").valid);
  ASSERT_TRUE(font->load(16, FontMethod::MSDF));
  const auto unicode = font->makeTextDrawData("A\xe3\x81\x82\xce\xa9");
  EXPECT_TRUE(unicode.valid);
}

TEST_F(TextCacheTests, GpuCacheIsBoundedAndDoesNotKeepDeadFontsAlive) {
  TextModelCache cache;
  auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
  const auto seed = Rect(0, 0, 20, 10).createMesh();
  Array<Mesh> sources;
  for (Size i = 0; i < TextModelCache::maxEntries + 4; ++i) {
    sources.emplace_back(*seed.data());
    cache.acquire(sources.back(), commands);
    EXPECT_LE(cache.size(), TextModelCache::maxEntries);
    EXPECT_LE(cache.byteSize(), TextModelCache::maxBytes);
  }
  EXPECT_EQ(cache.size(), TextModelCache::maxEntries);
  sources.clear();
  cache.acquire(seed, commands);
  EXPECT_EQ(cache.size(), 1u);
  cache.clear();
  EXPECT_EQ(cache.byteSize(), 0u);
  Array<Mesh> largeSources;
  for (int i = 0; i < 5; ++i) {
    auto data = makePtr<Mesh::Data>(GlobalAllocator::get());
    data->vertices.resize(TextModelCache::maxBytes / 4 / sizeof(Vertex));
    data->indices = {0, 1, 2};
    largeSources.emplace_back(data);
    cache.acquire(largeSources.back(), commands);
    EXPECT_LE(cache.byteSize(), TextModelCache::maxBytes);
  }
  EXPECT_LT(cache.size(), largeSources.size());
  device->submitCommandBuffer(commands);
}

TEST_F(TextCacheTests, FontAtlasIsRebuiltForAReplacementDevice) {
  auto font = Font::create();
  ASSERT_TRUE(font->load(12, FontMethod::Bitmap));
  const auto before = font->makeTextDrawData("same text");
  ASSERT_TRUE(before.valid);
  Graphics::shutdown();
  ASSERT_TRUE(Graphics::initialize(GPUBackendAPI::D3D12));
  const auto after = font->makeTextDrawData("same text");
  ASSERT_TRUE(after.valid);
  EXPECT_NE(before.mesh.data(), after.mesh.data());
  EXPECT_NE(before.texture->getRaw(), after.texture->getRaw());
  Graphics::getDevice()->waitForGpuIdle();
  // Exercise the portable text-cache fallback and the same resource teardown
  // order used by backend switching: unload scene/font resources first.
  for (const auto api : {GPUBackendAPI::Vulkan, GPUBackendAPI::D3D12}) {
    font->unload();
    Graphics::shutdown();
    ASSERT_TRUE(Graphics::initialize(api));
    ASSERT_TRUE(font->load(12, FontMethod::Bitmap));
    {
      const auto text = font->makeTextDrawData("backend switch");
      ASSERT_TRUE(text.valid);
      auto current = Graphics::getDevice();
      auto commands = current->acquireCommandBuffer({GlobalAllocator::get()});
      TextModelCache cache;
      const auto model = cache.acquire(text.mesh, commands);
      EXPECT_NE(model->vertexBuffer, nullptr);
      current->submitCommandBuffer(commands);
      current->waitForGpuIdle();
    }
  }
}

TEST_F(TextCacheTests, StreamedAndCachedTextMatchThroughMsaaResizeAndReadback) {
  auto font = Font::create();
  ASSERT_TRUE(font->load(12, FontMethod::Bitmap));
  TextStyle style(font, Color(1.0f), 12.0f);
  RenderTexture target;
  auto output = Texture::create(64, 64);
  Graphics::addPostDrawFunc([&] {
    auto pass = Graphics::begin2DPass(target);
    ASSERT_NE(pass, nullptr);
    pass->drawRect(Rect(0, 0, 64, 64), Color(1, 0, 0, 1));
    pass->drawText("Cache", style, TextTransform(Vec2(4, 8)));
    Array<TextBatchItem> batch;
    batch.push_back({"12", TextTransform(Vec2(4, 30))});
    batch.push_back({"34", TextTransform(Vec2(28, 30))});
    pass->drawTexts(batch, style);
    Graphics::endPass(pass);
  });
  for (UInt32 samples : {1u, 4u}) {
    Graphics::setMSAASampleCount(samples);
    Window::resize(samples == 1 ? Vec2(320, 240) : Vec2(480, 320));
    SDL_PumpEvents();
    target.create(64, 64);
    std::vector<UInt8> reference;
    for (int frame = 0; frame < 4; ++frame) {
      Graphics::render();
      ASSERT_TRUE(Graphics::readbackTexture(target, output));
      const auto png = output->toPngBuffer();
      int width = 0, height = 0, channels = 0;
      std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> decoded(
          stbi_load_from_memory(static_cast<const stbi_uc *>(png.data()),
                                static_cast<int>(png.size()), &width, &height,
                                &channels, 4), stbi_image_free);
      ASSERT_NE(decoded, nullptr);
      ASSERT_EQ(width, 64);
      ASSERT_EQ(height, 64);
      const std::vector<UInt8> pixels(decoded.get(), decoded.get() + width * height * 4);
      EXPECT_EQ(pixels[0], 255u);
      EXPECT_EQ(pixels[1], 0u);
      EXPECT_EQ(pixels[2], 0u);
      if (reference.empty()) {
        reference = pixels;
      } else {
        EXPECT_EQ(pixels, reference);
      }
    }
  }
}
