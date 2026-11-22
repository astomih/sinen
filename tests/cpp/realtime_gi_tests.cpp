#define NOMINMAX
#include "gpu/backend/d3d12/d3d12_device.hpp"
#include "platform/window/window_native.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <d3d12sdklayers.h>
#include <graphics/graphics.hpp>
#include <graphics/model/mesh_builder.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <platform/window/window.hpp>
#include <renderer/global_illumination.hpp>

using namespace sinen;
namespace {
class RealtimeGITests : public testing::TestWithParam<GPUBackendAPI> {
protected:
  void SetUp() override {
    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
    ASSERT_TRUE(Window::initialize("Sinen real-time GI test"));
    SDL_HideWindow(static_cast<SDL_Window *>(WindowNative::getWindow()));
    ASSERT_TRUE(Graphics::initialize(GetParam()));
    device = Graphics::getDevice();
    if (GetParam() == GPUBackendAPI::D3D12)
      std::static_pointer_cast<gpu::d3d12::Device>(device)
          ->getNative()
          ->QueryInterface(IID_PPV_ARGS(&messages));
    if (messages)
      messages->ClearStoredMessages();
  }
  void TearDown() override {
    if (device)
      device->waitForGpuIdle();
    Graphics::shutdown();
    if (messages)
      for (UINT64 i = 0; i < messages->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0;
        messages->GetMessage(i, nullptr, &size);
        std::vector<char> data(size);
        auto *message = reinterpret_cast<D3D12_MESSAGE *>(data.data());
        if (SUCCEEDED(messages->GetMessage(i, message, &size)) &&
            message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
          ADD_FAILURE() << message->pDescription;
      }
    device.reset();
    messages.Reset();
    Window::shutdown();
    SDL_Quit();
  }
  Vec3 readMean(const GlobalIllumination &gi) {
    auto texture = gi.getTexture()->getRaw();
    const auto info = texture->getCreateInfo();
    const UInt32 row = GetParam() == GPUBackendAPI::Vulkan
                           ? info.width * 16
                           : (info.width * 16 + 255) & ~255u;
    auto download = device->createTransferBuffer(
        {GlobalAllocator::get(), gpu::TransferBufferUsage::Download,
         row * info.height});
    auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
    auto copy = commands->beginCopyPass();
    gpu::TextureRegion region{};
    region.texture = texture;
    region.width = info.width;
    region.height = info.height;
    region.depth = 1;
    copy->downloadTexture(region, {download, 0});
    commands->endCopyPass(copy);
    device->submitCommandBuffer(commands);
    device->waitForGpuIdle();
    const auto *bytes = static_cast<const char *>(download->map(false));
    if (!bytes) {
      ADD_FAILURE() << "GI atlas readback failed";
      return Vec3(0);
    }
    Vec3 result;
    for (UInt32 y = 0; y < info.height; ++y)
      for (UInt32 x = 0; x < info.width; ++x) {
        float rgba[4];
        std::memcpy(rgba, bytes + y * row + x * 16, sizeof(rgba));
        result += Vec3(rgba[0], rgba[1], rgba[2]);
      }
    download->unmap();
    return result * (1.0f / (info.width * info.height));
  }
  Model box() {
    MeshBuilder builder;
    builder.addBox(Vec3(4));
    Model model;
    *model.getMesh().data() = *builder.toMesh().data();
    return model;
  }
  Ptr<gpu::Device> device;
  gpu::d3d12::ComPtr<ID3D12InfoQueue> messages;
};
} // namespace

TEST_P(RealtimeGITests, UpdatesSkyOnGpuWithoutReinitializingTheVolume) {
  GlobalIllumination gi;
  EXPECT_FALSE(gi.initializeRealtime(Vec3(1), Vec3(-1)));
  EXPECT_FALSE(gi.initializeRealtime(Vec3(-1), Vec3(1), 1));
  gi.setSkyColor(Color(0.2f, 0.4f, 0.8f, 1.0f));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.isRealtime());
  const auto atlas = gi.getTexture();
  ASSERT_TRUE(gi.update(0));
  const auto first = readMean(gi);
  EXPECT_NEAR(first.x, 0.2f * 3.14159f, 0.06f);
  EXPECT_NEAR(first.y, 0.4f * 3.14159f, 0.12f);
  EXPECT_NEAR(first.z, 0.8f * 3.14159f, 0.24f);
  gi.setSkyColor(Color(0.8f, 0.1f, 0.05f, 1.0f));
  ASSERT_TRUE(gi.update(0));
  const auto second = readMean(gi);
  EXPECT_EQ(gi.getTexture(), atlas);
  EXPECT_GT(second.x, first.x * 3);
  EXPECT_LT(second.z, first.z * 0.1f);
  EXPECT_FALSE(gi.update(std::numeric_limits<float>::quiet_NaN()));
  gi.clear();
  EXPECT_FALSE(gi.isRealtime());
  EXPECT_FALSE(gi.update());
}

TEST_P(RealtimeGITests, MovingGeometryChangesEmissionAndOcclusionNextFrame) {
  GlobalIllumination gi;
  gi.setSkyColor(Color(0, 0, 1, 1));
  auto model = box();
  gi.addModel(model, Transform{}, Color(0), Color(1, 0, 0, 1));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update(0));
  const auto inside = readMean(gi);
  EXPECT_GT(inside.x, 2.5f);
  EXPECT_FLOAT_EQ(inside.z, 0.0f);
  Transform moved;
  moved.position = Vec3(10, 0, 0);
  gi.beginFrame();
  gi.addModel(model, moved, Color(0), Color(1, 0, 0, 1));
  ASSERT_TRUE(gi.update(0));
  const auto outside = readMean(gi);
  EXPECT_LT(outside.x, 0.2f);
  EXPECT_GT(outside.z, 2.5f);
}

TEST_P(RealtimeGITests, MovingLightChangesReflectedLightAndShadows) {
  GlobalIllumination gi;
  gi.addModel(box(), Transform{});
  gi.setLight(PointLight(Vec3(0), 1));
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update(0));
  const auto inside = readMean(gi);
  EXPECT_GT(inside.x, 0.1f);
  gi.setLight(PointLight(Vec3(10, 0, 0), 1));
  ASSERT_TRUE(gi.update(0));
  const auto outside = readMean(gi);
  EXPECT_LT(outside.x, inside.x * 0.01f);
}

TEST_P(RealtimeGITests, BlendsHistoryAndCanRespondImmediately) {
  GlobalIllumination gi;
  ASSERT_TRUE(gi.initializeRealtime(Vec3(-1), Vec3(1), 2, 2, 2));
  ASSERT_TRUE(gi.update());
  EXPECT_FLOAT_EQ(readMean(gi).x, 0.0f);
  gi.setSkyColor(Color(1));
  ASSERT_TRUE(gi.update(0.5f));
  const auto smoothed = readMean(gi);
  EXPECT_NEAR(smoothed.x, 3.14159f * 0.5f, 0.12f);
  ASSERT_TRUE(gi.update(0));
  const auto immediate = readMean(gi);
  EXPECT_NEAR(immediate.x, 3.14159f, 0.24f);
  EXPECT_GT(immediate.x, smoothed.x * 1.8f);
}

TEST_P(RealtimeGITests, RejectsInvalidGpuBufferTextureCopies) {
  auto buffer = device->createBuffer(
      {GlobalAllocator::get(), gpu::BufferUsage::Storage, 512});
  auto texture = Texture::create(2, 2);
  auto commands = device->acquireCommandBuffer({GlobalAllocator::get()});
  auto copy = commands->beginCopyPass();
  gpu::TextureRegion region{};
  region.texture = texture->getRaw();
  region.width = 2;
  region.height = 2;
  region.depth = 1;
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 1, 511}, region, 256));
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 512}, region, 255));
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 1}, region, 256));
  EXPECT_FALSE(copy->copyBufferToTexture({nullptr, 0, 512}, region, 256));
  region.depth = 2;
  EXPECT_FALSE(copy->copyBufferToTexture({buffer, 0, 512}, region, 256));
  commands->endCopyPass(copy);
  device->submitCommandBuffer(commands);
}

const GPUBackendAPI giTestBackends[] = {GPUBackendAPI::D3D12,
                                        GPUBackendAPI::Vulkan
#ifdef SINEN_ENABLE_WEBGPU
                                        ,
                                        GPUBackendAPI::WebGPU
#endif
};
INSTANTIATE_TEST_SUITE_P(Backends, RealtimeGITests,
                         testing::ValuesIn(giTestBackends));
