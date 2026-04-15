#ifndef SINEN_D3D12_DEVICE_HPP
#define SINEN_D3D12_DEVICE_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_resources.hpp"

#include <gpu/gpu_device.hpp>

#include <SDL3/SDL_video.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <vector>

namespace sinen::gpu::d3d12 {
class CommandBuffer;

class Device : public gpu::Device {
public:
  explicit Device(const CreateInfo &createInfo);
  ~Device() override;

  void claimWindow(void *window) override;
  Ptr<gpu::Buffer>
  createBuffer(const gpu::Buffer::CreateInfo &createInfo) override;
  Ptr<gpu::Texture>
  createTexture(const gpu::Texture::CreateInfo &createInfo) override;
  Ptr<gpu::Sampler>
  createSampler(const gpu::Sampler::CreateInfo &createInfo) override;
  Ptr<gpu::TransferBuffer>
  createTransferBuffer(const gpu::TransferBuffer::CreateInfo &createInfo) override;
  Ptr<gpu::Shader>
  createShader(const gpu::Shader::CreateInfo &createInfo) override;
  Ptr<gpu::GraphicsPipeline> createGraphicsPipeline(
      const gpu::GraphicsPipeline::CreateInfo &createInfo) override;
  Ptr<gpu::ComputePipeline>
  createComputePipeline(const gpu::ComputePipeline::CreateInfo &createInfo) override;
  Ptr<gpu::CommandBuffer>
  acquireCommandBuffer(const gpu::CommandBuffer::CreateInfo &createInfo) override;
  void submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) override;
  Ptr<gpu::Texture>
  acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) override;
  gpu::TextureFormat getSwapchainFormat() const override;
  void waitForGpuIdle() override;
  String getDriver() const override;
  GPUBackendAPI getBackendAPI() const override { return GPUBackendAPI::D3D12U; }

  std::shared_ptr<Device> get() {
    return std::dynamic_pointer_cast<Device>(getPtr());
  }

  ID3D12Device *getNative() const { return device.Get(); }
  ID3D12CommandQueue *getQueue() const { return commandQueue.Get(); }
  ID3D12DescriptorHeap *getSrvHeap() const { return srvHeap.Get(); }
  ID3D12DescriptorHeap *getSamplerHeap() const { return samplerHeap.Get(); }
  UINT getSrvDescriptorSize() const { return srvDescriptorSize; }
  UINT getSamplerDescriptorSize() const { return samplerDescriptorSize; }

  CpuGpuDescriptor allocateSrvDescriptor();
  CpuGpuDescriptor allocateSamplerDescriptor();
  D3D12_CPU_DESCRIPTOR_HANDLE allocateRtvDescriptor();
  D3D12_CPU_DESCRIPTOR_HANDLE allocateDsvDescriptor();
  CpuGpuDescriptor allocateTransientSrvDescriptor();
  CpuGpuDescriptor allocateTransientSamplerDescriptor();
  CpuGpuDescriptor getDefaultSrv() const { return defaultSrv; }
  CpuGpuDescriptor getDefaultSampler() const { return defaultSampler; }
  void resetTransientDescriptors();

  ID3D12RootSignature *getGraphicsRootSignature();
  void transition(ID3D12GraphicsCommandList *list, Texture *texture,
                  D3D12_RESOURCE_STATES after);
  void transition(ID3D12GraphicsCommandList *list, Buffer *buffer,
                  D3D12_RESOURCE_STATES after);

private:
  static constexpr UINT FrameCount = 2;
  static constexpr UINT SrvHeapCapacity = 4096;
  static constexpr UINT SamplerHeapCapacity = 256;
  static constexpr UINT RtvHeapCapacity = 1024;
  static constexpr UINT DsvHeapCapacity = 256;
  static constexpr UINT TransientSrvBase = 2048;
  static constexpr UINT TransientSamplerBase = 128;

  void createDeviceObjects();
  void createSwapchain();
  void destroySwapchain();
  void createDefaultDescriptors();
  void createGraphicsRootSignature();
  void signalAndWait();
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(ID3D12DescriptorHeap *heap,
                                        UINT descriptorSize, UINT index) const;
  D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle(ID3D12DescriptorHeap *heap,
                                        UINT descriptorSize, UINT index) const;

  SDL_Window *window = nullptr;
  void *hwnd = nullptr;

  Microsoft::WRL::ComPtr<IDXGIFactory6> factory;
  Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
  Microsoft::WRL::ComPtr<ID3D12Device> device;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
  Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;
  DXGI_FORMAT swapchainFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
  UINT width = 0;
  UINT height = 0;
  UINT currentBackBuffer = 0;
  std::vector<Ptr<Texture>> swapchainTextures;

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvCpuHeap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerCpuHeap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
  UINT srvDescriptorSize = 0;
  UINT samplerDescriptorSize = 0;
  UINT rtvDescriptorSize = 0;
  UINT dsvDescriptorSize = 0;
  UINT srvUsed = 0;
  UINT samplerUsed = 0;
  UINT rtvUsed = 0;
  UINT dsvUsed = 0;
  UINT transientSrvUsed = TransientSrvBase;
  UINT transientSamplerUsed = TransientSamplerBase;
  CpuGpuDescriptor defaultSrv{};
  CpuGpuDescriptor defaultSampler{};

  Microsoft::WRL::ComPtr<ID3D12RootSignature> graphicsRootSignature;
  Microsoft::WRL::ComPtr<ID3D12Fence> fence;
  UINT64 fenceValue = 0;
  void *fenceEvent = nullptr;
};
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_DEVICE_HPP
