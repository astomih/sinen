#ifndef PARANOIXA_WEBGPU_RENDERER_HPP
#define PARANOIXA_WEBGPU_RENDERER_HPP
#if 0

#ifndef __ANDROID__

#include <SDL3/SDL.h>
#include <graphics/paranoixa/paranoixa.hpp>
#include <webgpu/webgpu.h>

namespace sinen::rhi {
class WebGPURenderer {
public:
  WebGPURenderer(Allocator *pAllocator);
  ~WebGPURenderer();
  void Initialize(void *window);
  void ProcessEvent(void *event);
  void BeginFrame();
  void EndFrame();

  void AddGuiUpdateCallBack(std::function<void()> callBack);
  class Texture {
  public:
    Texture() = default;
    ~Texture() = default;
    WGPUTexture texture;
    WGPUTextureView view;
  };

private:
  void PrepareSurface(void *window);
  void PrepareInstance();
  void PrepareAdapter();
  void PrepareDevice();
  void PrepareQueue();

  void ConfigSurface(uint32_t width, uint32_t height);
  Texture CreateTexture(const void *data, size_t size, int width, int height);
  WGPUBuffer CreateBuffer(uint64_t size, WGPUBufferUsage usage);
  void PrepareSampler();
  void InitializePipeline();

  WGPUTextureView GetNextSurfaceTextureView();
#ifdef DAWN
  WGPUStringView GetStringView(const char *str);
#else
  const char *GetStringView(const char *str);
#endif

  // WebGPU instance
  WGPUInstance instance;
  // WebGPU adapter
  WGPUAdapter adapter;
  // WebGPU device
  WGPUDevice device;
  // WebGPU queue
  WGPUQueue queue;
  // WebGPU surface
  WGPUSurface surface;
  // WebGPU surface texture view
  WGPUTextureView targetView;
  WGPUBindGroup bindGroup;

  WGPUTextureFormat surfaceFormat;
  Texture texture;
  WGPUSampler sampler;
  WGPURenderPipeline pipeline;
  WGPUBuffer vertexBuffer;
  WGPUCommandEncoder encoder;
};
} // namespace paranoixa
#endif // __ANDROID__
#endif
#endif // PARANOIXA_WEBGPU_RENDERER_HPP