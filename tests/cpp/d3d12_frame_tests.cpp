#include "gpu/backend/d3d12/d3d12_command.hpp"
#include "gpu/backend/d3d12/d3d12_device.hpp"

#include <SDL3/SDL.h>
#include <d3d12sdklayers.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <set>
#include <thread>

using namespace sinen;
namespace dx = sinen::gpu::d3d12;

namespace {
class D3D12Frames : public testing::Test {
protected:
  void SetUp() override {
    allocator = std::pmr::new_delete_resource();
    device = makePtr<dx::Device>(allocator,
                                 gpu::Device::CreateInfo{allocator, true});
    if (!device->isValid()) {
      GTEST_SKIP() << "No D3D12 adapter available";
    }
    device->getNative()->QueryInterface(IID_PPV_ARGS(&messages));
    if (messages) {
      messages->ClearStoredMessages();
    }
  }
  void TearDown() override {
    if (device && device->isValid()) {
      device->waitForGpuIdle();
    }
    if (messages) {
      for (UINT64 i = 0; i < messages->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0;
        messages->GetMessage(i, nullptr, &size);
        std::vector<char> storage(size);
        auto *message = reinterpret_cast<D3D12_MESSAGE *>(storage.data());
        if (SUCCEEDED(messages->GetMessage(i, message, &size)) &&
            message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR) {
          ADD_FAILURE() << message->pDescription;
        }
      }
    }
    device.reset();
    messages.Reset();
  }
  Allocator *allocator = nullptr;
  Ptr<dx::Device> device;
  dx::ComPtr<ID3D12InfoQueue> messages;
};

// A watchdog releases a GPU gate even if a regression blocks submit(). The
// test checks whether CPU submission finished before that watchdog was needed.
class QueueGate {
public:
  explicit QueueGate(const Ptr<dx::Device> &device) {
    EXPECT_TRUE(SUCCEEDED(device->getNative()->CreateFence(
        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
    EXPECT_TRUE(SUCCEEDED(device->getQueue()->Wait(fence.Get(), 1)));
    watchdog = std::thread([this] {
      std::unique_lock lock(mutex);
      if (!changed.wait_for(lock, std::chrono::seconds(5),
                            [this] { return released; })) {
        timedOut = true;
      }
      fence->Signal(1);
    });
  }
  void release() {
    {
      std::lock_guard lock(mutex);
      released = true;
    }
    changed.notify_one();
    if (watchdog.joinable()) {
      watchdog.join();
    }
  }
  ~QueueGate() { release(); }
  bool timedOut = false;

private:
  dx::ComPtr<ID3D12Fence> fence;
  std::mutex mutex;
  std::condition_variable changed;
  bool released = false;
  std::thread watchdog;
};
} // namespace

TEST_F(D3D12Frames,
       InFlightUploadsSurviveWrapperReleaseAndReuseCompletedStorage) {
  std::array<Ptr<gpu::TransferBuffer>, 3> downloads;
  std::set<const dx::CommandContext *> contexts;
  {
    QueueGate gate(device);
    for (UInt32 i = 0; i < downloads.size(); ++i) {
      auto commands = device->acquireCommandBuffer({allocator});
      ASSERT_NE(commands, nullptr);
      auto native = std::static_pointer_cast<dx::CommandBuffer>(commands);
      contexts.insert(native->getContext().get());
      std::array<UInt32, 16> data;
      data.fill(0x11223300 + i);
      gpu::Buffer::CreateInfo info{allocator, gpu::BufferUsage::Vertex,
                                   sizeof(data)};
      auto first = commands->createTransientBuffer(info, data.data());
      data.fill(0xaabbcc00 + i);
      auto second = commands->createTransientBuffer(info, data.data());
      ASSERT_NE(first, nullptr);
      ASSERT_NE(second, nullptr);
      commands->pushVertexUniformData(0, data.data(), sizeof(data));
      EXPECT_EQ(native->vertexUniform(0) % 256, 0u);
      downloads[i] = device->createTransferBuffer(
          {allocator, gpu::TransferBufferUsage::Download, 2 * sizeof(data)});
      ASSERT_NE(downloads[i], nullptr);
      auto pass = commands->beginCopyPass();
      pass->downloadBuffer({first, 0, sizeof(data)}, {downloads[i], 0});
      pass->downloadBuffer({second, 0, sizeof(data)},
                           {downloads[i], sizeof(data)});
      commands->endCopyPass(pass);
      device->submitCommandBuffer(commands);
      // All recording wrappers and temporary buffers die while GPU is gated.
    }
    gate.release();
    EXPECT_FALSE(gate.timedOut)
        << "submitCommandBuffer blocked on GPU completion";
  }
  EXPECT_EQ(contexts.size(), 3u);
  device->waitForGpuIdle();
  for (UInt32 i = 0; i < downloads.size(); ++i) {
    auto *values = static_cast<UInt32 *>(downloads[i]->map(false));
    ASSERT_NE(values, nullptr);
    for (Size j = 0; j < 16; ++j) {
      EXPECT_EQ(values[j], 0x11223300 + i);
      EXPECT_EQ(values[j + 16], 0xaabbcc00 + i);
    }
    downloads[i]->unmap();
  }
  for (int i = 0; i < 8; ++i) {
    auto commands = device->acquireCommandBuffer({allocator});
    auto native = std::static_pointer_cast<dx::CommandBuffer>(commands);
    EXPECT_TRUE(contexts.contains(native->getContext().get()));
    const auto *page = native->getContext()->pages.front().resource.Get();
    std::array<UInt32, 16> data{};
    commands->pushVertexUniformData(0, data.data(), sizeof(data));
    EXPECT_EQ(page, native->getContext()->pages.front().resource.Get());
    EXPECT_EQ(native->getContext()->pages.size(), 1u);
    device->submitCommandBuffer(commands);
    device->waitForGpuIdle();
  }
}

TEST_F(D3D12Frames, FrameDescriptorStorageIsIsolatedAndRecycled) {
  device->beginFrame();
  auto *firstHeap = device->getSrvHeap();
  const auto first = device->allocateTransientSrvDescriptor();
  auto commands = device->acquireCommandBuffer({allocator});
  device->submitCommandBuffer(commands);
  commands.reset();
  device->endFrame();
  device->beginFrame();
  EXPECT_NE(firstHeap, device->getSrvHeap());
  EXPECT_NE(first.gpu.ptr, device->allocateTransientSrvDescriptor().gpu.ptr);
  commands = device->acquireCommandBuffer({allocator});
  device->submitCommandBuffer(commands);
  commands.reset();
  device->endFrame();
  device->beginFrame();
  EXPECT_EQ(firstHeap, device->getSrvHeap());
  EXPECT_EQ(first.gpu.ptr, device->allocateTransientSrvDescriptor().gpu.ptr);
  device->endFrame();
}

TEST_F(D3D12Frames, CpuDescriptorsAreRecycledForTransientTexturesAndSamplers) {
  gpu::Texture::CreateInfo info{};
  info.allocator = allocator;
  info.type = gpu::TextureType::Texture2D;
  info.format = gpu::TextureFormat::R8G8B8A8_UNORM;
  info.usage = gpu::TextureUsage::Sampler;
  info.width = info.height = info.layerCountOrDepth = info.numLevels = 1;
  info.sampleCount = gpu::SampleCount::x1;
  auto first =
      std::static_pointer_cast<dx::Texture>(device->createTexture(info));
  ASSERT_NE(first, nullptr);
  const auto handle = first->getSrvCpu().ptr;
  first.reset();
  for (int i = 0; i < 256; ++i) {
    auto texture =
        std::static_pointer_cast<dx::Texture>(device->createTexture(info));
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(handle, texture->getSrvCpu().ptr);
  }
  gpu::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = allocator;
  auto sampler =
      std::static_pointer_cast<dx::Sampler>(device->createSampler(samplerInfo));
  ASSERT_NE(sampler, nullptr);
  const auto samplerHandle = sampler->getCpu().ptr;
  sampler.reset();
  for (int i = 0; i < 256; ++i) {
    auto next = std::static_pointer_cast<dx::Sampler>(
        device->createSampler(samplerInfo));
    ASSERT_NE(next, nullptr);
    EXPECT_EQ(next->getCpu().ptr, samplerHandle);
  }
}

TEST_F(D3D12Frames,
       SwapchainResizeWorksWithSubmittedFramesAndRetainedResources) {
  ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));
  auto *window = SDL_CreateWindow("Sinen frame recycling test", 128, 96,
                                  SDL_WINDOW_HIDDEN);
  ASSERT_NE(window, nullptr) << SDL_GetError();
  device->claimWindow(window);
  for (int i = 0; i < 8; ++i) {
    if (i == 4) {
      ASSERT_TRUE(SDL_SetWindowSize(window, 192, 128));
      SDL_PumpEvents();
    }
    device->beginFrame();
    auto commands = device->acquireCommandBuffer({allocator});
    auto texture = device->acquireSwapchainTexture(commands);
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->getCreateInfo().width, i < 4 ? 128u : 192u);
    gpu::ColorTargetInfo target{};
    target.texture = texture;
    target.loadOp = gpu::LoadOp::Clear;
    target.storeOp = gpu::StoreOp::Store;
    Array<gpu::ColorTargetInfo> targets(allocator);
    targets.push_back(target);
    auto pass = commands->beginRenderPass(targets, {}, 0.2f, 0.4f, 0.6f, 1.0f);
    commands->endRenderPass(pass);
    device->submitCommandBuffer(commands);
    device->endFrame();
  }
  device->releaseResources();
  SDL_DestroyWindow(window);
  SDL_Quit();
}
