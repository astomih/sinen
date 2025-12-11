#ifdef _WIN32
#ifndef SINEN_D3D12U_RENDERER_HPP
#define SINEN_D3D12U_RENDERER_HPP
#include <d3d12.h>

#include <SDL3/SDL.h>

#include <graphics/rhi/rhi.hpp>

#include <dxgi1_6.h>

namespace sinen::rhi {
class D3d12uRenderer {
public:
  D3d12uRenderer(Allocator *allcator);
  ~D3d12uRenderer();
  void Initialize(void *window);
  void ProcessEvent(void *event);
  void BeginFrame();
  void EndFrame();

  void AddGuiUpdateCallBack(std::function<void()> callBack);

public:
  struct DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
    D3D12_DESCRIPTOR_HEAP_TYPE type;
  };

  ID3D12Resource1 *CreateBuffer(D3D12_RESOURCE_DESC desc,
                                D3D12_HEAP_PROPERTIES heapProperties);
  ID3D12Resource1 *CreateTexture(const void *data, size_t size, int width,
                                 int height);
  DescriptorHandle CreateSampler(const D3D12_SAMPLER_DESC &desc);
  ID3D12RootSignature *CreateRootSignature(ID3DBlob *signature);
  ID3D12PipelineState *
  CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc);
  ID3D12GraphicsCommandList *CreateCommandList();
  DescriptorHandle
  CreateShaderResourceView(ID3D12Resource1 *res,
                           D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc);
  DXGI_FORMAT GetSwapchainFormat() const { return DXGI_FORMAT_R8G8B8A8_UNORM; }

  UINT GetFrameIndex() const { return frameIndex; }
  DescriptorHandle GetSwapchainBufferDescriptor();
  ID3D12Resource1 *GetSwapchainBufferResource();

  void Submit(ID3D12CommandList *const commandList);
  void Present(UINT syncInterval, UINT flags = 0);

private:
  struct DescriptorHeapInfo {
    ID3D12DescriptorHeap *heap;
    UINT handleSize = 0;
    UINT usedIndex = 0;
    std::vector<DescriptorHandle> handles;
  };
  struct FrameInfo {
    UINT64 fenceValue = 0;
    ID3D12CommandAllocator *commandAllocator;
    DescriptorHandle rtvDescriptor;
    ID3D12Resource1 *targetBuffer;
  };
  void PrepareDevice();
  void PrepareCommandQueue();
  void PrepareDescriptorHeap();
  DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
  void DeallocateDescriptor(DescriptorHandle descriptor);
  ID3D12DescriptorHeap *GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
  DescriptorHeapInfo *GetDescriptorHeapInfo(D3D12_DESCRIPTOR_HEAP_TYPE);
  void PrepareCommandAllocator();
  void PrepareSwapChain();
  void PrepareRenderTargetView();

  void PrepareTriangle();
  void PrepareTexture();

  UINT frameCount = 2;
  Allocator *allocator;
  HWND hWindow;
  void *pWindow;
  ID3D12Device *device;
#ifdef _DEBUG
  ID3D12Debug *d3d12Debug;
  ID3D12Debug3 *d3d12Debug3;
#endif
  IDXGIFactory7 *dxgiFactory;
  IDXGIAdapter4 *adapter;
  ID3D12CommandQueue *commandQueue;
  DescriptorHeapInfo rtvDescriptorHeap;
  DescriptorHeapInfo dsvDescriptorHeap;
  DescriptorHeapInfo srvDescriptorHeap;
  DescriptorHeapInfo imguiDescriptorHeap;
  DescriptorHeapInfo samplerDescriptorHeap;
  ID3D12CommandAllocator *commandAllocator;
  void *waitFence;
  ID3D12Fence *frameFence;
  FrameInfo frameInfo[2];
  IDXGISwapChain4 *swapChain;
  int width, height;
  UINT frameIndex;

  ID3D12Resource1 *vertexBuffer;
  ID3D12Resource1 *texture;
  ID3D12RootSignature *rootSignature;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  ID3D12PipelineState *pipelineState;
  DescriptorHandle textureDescriptor;
  DescriptorHandle samplerDescriptor;
  std::vector<std::function<void()>> guiCallBacks;
};
} // namespace sinen::rhi
#endif // SINEN_D3D12U_RENDERER_HPP
#endif // EMSCRIPTEN