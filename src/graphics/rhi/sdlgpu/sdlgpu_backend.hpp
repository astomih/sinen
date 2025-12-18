#include <memory>
#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_RENDERER_HPP
#define SINEN_SDLGPU_RENDERER_HPP
#include <graphics/rhi/rhi.hpp>

#include <SDL3/SDL_gpu.h>

#include <vector>

namespace sinen::rhi::sdlgpu {
namespace px = sinen::rhi;
class Device : public px::Device {
public:
  Device(const CreateInfo &createInfo, SDL_GPUDevice *device)
      : px::Device(createInfo), device(device), window(nullptr) {}
  SDL_GPUDevice *getNative() { return device; }
  virtual ~Device() override;
  virtual void claimWindow(void *window) override;
  virtual Ptr<px::Buffer>
  createBuffer(const Buffer::CreateInfo &createInfo) override;
  virtual Ptr<px::Texture>
  createTexture(const Texture::CreateInfo &createInfo) override;
  virtual Ptr<px::Sampler>
  createSampler(const Sampler::CreateInfo &createInfo) override;
  virtual Ptr<px::TransferBuffer>
  createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) override;
  virtual Ptr<px::Shader>
  createShader(const Shader::CreateInfo &createInfo) override;
  virtual Ptr<px::CommandBuffer>
  acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) override;
  virtual Ptr<px::GraphicsPipeline> createGraphicsPipeline(
      const GraphicsPipeline::CreateInfo &createInfo) override;
  virtual Ptr<px::ComputePipeline>
  createComputePipeline(const ComputePipeline::CreateInfo &createInfo) override;
  virtual void
  submitCommandBuffer(Ptr<px::CommandBuffer> commandBuffer) override;
  virtual Ptr<px::Texture>
  acquireSwapchainTexture(Ptr<px::CommandBuffer> commandBuffer) override;
  virtual px::TextureFormat getSwapchainFormat() const override;
  virtual void waitForGpuIdle() override;
  virtual String getDriver() const override;
  std::shared_ptr<Device> get() {
    return std::dynamic_pointer_cast<Device>(getPtr());
  }

private:
  SDL_GPUDevice *device;
  SDL_Window *window;
};
class Texture : public px::Texture {
public:
  Texture(const CreateInfo &createInfo, const Ptr<Device> &device,
          SDL_GPUTexture *texture, bool isSwapchainTexture = false)
      : px::Texture(createInfo), device(device), texture(texture),
        isSwapchainTexture(isSwapchainTexture) {}
  virtual ~Texture() override;

  inline SDL_GPUTexture *getNative() const { return texture; }

private:
  Ptr<Device> device;
  SDL_GPUTexture *texture;
  bool isSwapchainTexture;
};

class Sampler : public px::Sampler {
public:
  Sampler(const CreateInfo &createInfo, const Ptr<Device> &device,
          SDL_GPUSampler *sampler)
      : px::Sampler(createInfo), device(device), sampler(sampler) {}
  ~Sampler() override;

  inline SDL_GPUSampler *getNative() const { return sampler; }

private:
  Ptr<Device> device;
  SDL_GPUSampler *sampler;
};

class TransferBuffer : public px::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, const Ptr<Device> &device,
                 SDL_GPUTransferBuffer *transferBuffer)
      : px::TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer) {}
  ~TransferBuffer() override;

  inline SDL_GPUTransferBuffer *getNative() { return transferBuffer; }

  void *map(bool cycle) override;
  void unmap() override;

private:
  Ptr<Device> device;
  SDL_GPUTransferBuffer *transferBuffer;
};
class Buffer : public px::Buffer {
public:
  Buffer(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUBuffer *buffer)
      : px::Buffer(createInfo), device(device), buffer(buffer) {}
  ~Buffer() override;

  inline SDL_GPUBuffer *getNative() { return buffer; }

private:
  Ptr<Device> device;
  SDL_GPUBuffer *buffer;
};
class Backend : public px::Backend {
public:
  virtual Ptr<px::Device>
  createDevice(const px::Device::CreateInfo &createInfo) override;
};
class Shader : public px::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUShader *shader)
      : px::Shader(createInfo), device(device), shader(shader) {}
  ~Shader() override;

  inline SDL_GPUShader *getNative() { return shader; }

private:
  Ptr<Device> device;
  SDL_GPUShader *shader;
};

class CopyPass : public px::CopyPass {
public:
  CopyPass(Allocator *allocator, class CommandBuffer &commandBuffer,
           SDL_GPUCopyPass *copyPass)
      : px::CopyPass(), commandBuffer(commandBuffer), copyPass(copyPass) {}
  inline SDL_GPUCopyPass *getNative() { return copyPass; }

  virtual void uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) override;
  virtual void downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) override;
  virtual void uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) override;
  virtual void downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) override;

  virtual void copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, uint32 width,
                           uint32 height, uint32 depth, bool cycle) override;

private:
  SDL_GPUCopyPass *copyPass;
  class CommandBuffer &commandBuffer;
};

class RenderPass : public px::RenderPass {
public:
  RenderPass(Allocator *allocator, CommandBuffer &commandBuffer,
             SDL_GPURenderPass *renderPass)
      : px::RenderPass(), allocator(allocator), commandBuffer(commandBuffer),
        renderPass(renderPass) {}

  inline SDL_GPURenderPass *getNative() const { return renderPass; }

  void bindGraphicsPipeline(Ptr<px::GraphicsPipeline> pipeline) override;
  void bindVertexBuffers(uint32 startSlot,
                         const Array<BufferBinding> &bindings) override;
  void bindIndexBuffer(const BufferBinding &binding,
                       IndexElementSize indexElementSize) override;
  void
  bindFragmentSamplers(uint32 startSlot,
                       const Array<TextureSamplerBinding> &bindings) override;
  void setViewport(const Viewport &viewport) override;
  void setScissor(int32 x, int32 y, int32 width, int32 height) override;
  void drawPrimitives(uint32 vertexCount, uint32 instanceCount,
                      uint32 firstVertex, uint32 firstInstance) override;
  void drawIndexedPrimitives(uint32 indexCount, uint32 instanceCount,
                             uint32 firstIndex, uint32 vertexOffset,
                             uint32 firstInstance) override;

private:
  Allocator *allocator;
  SDL_GPURenderPass *renderPass;
  class CommandBuffer &commandBuffer;
};

class CommandBuffer : public px::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo,
                SDL_GPUCommandBuffer *commandBuffer)
      : px::CommandBuffer(createInfo), commandBuffer(commandBuffer) {}

  SDL_GPUCommandBuffer *getNative() { return commandBuffer; }

  Ptr<px::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<px::CopyPass> copyPass) override;
  Ptr<px::RenderPass>
  beginRenderPass(const Array<px::ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<px::RenderPass> renderPass) override;

  void pushUniformData(uint32 slot, const void *data, size_t size) override;

private:
  SDL_GPUCommandBuffer *commandBuffer;
};

class GraphicsPipeline : public px::GraphicsPipeline {
public:
  GraphicsPipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                   SDL_GPUGraphicsPipeline *pipeline)
      : px::GraphicsPipeline(createInfo), device(device), pipeline(pipeline) {}
  ~GraphicsPipeline() override;

  inline SDL_GPUGraphicsPipeline *getNative() { return pipeline; }

private:
  Ptr<Device> device;
  SDL_GPUGraphicsPipeline *pipeline;
};

class ComputePipeline : public px::ComputePipeline {
public:
  ComputePipeline() : px::ComputePipeline({}) {};
  ComputePipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                  SDL_GPUComputePipeline *pipeline)
      : px::ComputePipeline(createInfo), device(device), pipeline(pipeline) {}
  ~ComputePipeline() override {}

private:
  Ptr<Device> device;
  SDL_GPUComputePipeline *pipeline;
};

} // namespace sinen::rhi::sdlgpu
#endif // SINEN_SDLGPU_RENDERER_HPP
#endif // EMSCRIPTEN