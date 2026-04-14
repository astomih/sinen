#ifndef SINEN_D3D12_COMMAND_HPP
#define SINEN_D3D12_COMMAND_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_resources.hpp"

#include <gpu/gpu_command_buffer.hpp>
#include <gpu/gpu_copy_pass.hpp>
#include <gpu/gpu_render_pass.hpp>

#include <array>
#include <vector>

namespace sinen::gpu::d3d12 {
class Device;

class CommandBuffer : public gpu::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo, Ptr<Device> device,
                ComPtr<ID3D12CommandAllocator> allocator,
                ComPtr<ID3D12GraphicsCommandList> commandList);

  ID3D12GraphicsCommandList *getNative() const { return commandList.Get(); }
  Ptr<Device> getDevice() const { return device; }
  void close();
  bool usesSwapchain() const { return swapchainUsed; }
  void markUsesSwapchain() { swapchainUsed = true; }

  Ptr<gpu::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<gpu::CopyPass> copyPass) override;
  Ptr<gpu::RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<gpu::RenderPass> renderPass) override;
  void pushVertexUniformData(UInt32 slot, const void *data,
                             size_t size) override;
  void pushFragmentUniformData(UInt32 slot, const void *data,
                               size_t size) override;

  D3D12_GPU_VIRTUAL_ADDRESS vertexUniform(UInt32 slot) const;
  D3D12_GPU_VIRTUAL_ADDRESS fragmentUniform(UInt32 slot) const;
  void keepAlive(ComPtr<ID3D12Resource> resource) { uploadResources.push_back(resource); }

private:
  D3D12_GPU_VIRTUAL_ADDRESS uploadUniform(const void *data, size_t size);

  Ptr<Device> device;
  ComPtr<ID3D12CommandAllocator> allocator;
  ComPtr<ID3D12GraphicsCommandList> commandList;
  bool closed = false;
  bool swapchainUsed = false;
  std::array<D3D12_GPU_VIRTUAL_ADDRESS, 4> vertexUniforms{};
  std::array<D3D12_GPU_VIRTUAL_ADDRESS, 4> fragmentUniforms{};
  std::vector<ComPtr<ID3D12Resource>> uploadResources;
};

class CopyPass : public gpu::CopyPass {
public:
  explicit CopyPass(CommandBuffer *commandBuffer)
      : commandBuffer(commandBuffer) {}

  void uploadTexture(const TextureTransferInfo &src, const TextureRegion &dst,
                     bool cycle) override;
  void downloadTexture(const TextureRegion &src,
                       const TextureTransferInfo &dst) override;
  void uploadBuffer(const BufferTransferInfo &src, const BufferRegion &dst,
                    bool cycle) override;
  void downloadBuffer(const BufferRegion &src,
                      const BufferTransferInfo &dst) override;
  void copyTexture(const TextureLocation &src, const TextureLocation &dst,
                   UInt32 width, UInt32 height, UInt32 depth,
                   bool cycle) override;

private:
  CommandBuffer *commandBuffer;
};

class RenderPass : public gpu::RenderPass {
public:
  RenderPass(CommandBuffer *commandBuffer,
             const Array<ColorTargetInfo> &colorTargets,
             const DepthStencilTargetInfo &depthStencilInfo);

  void bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> graphicsPipeline) override;
  void bindVertexBuffers(UInt32 slot,
                         const Array<BufferBinding> &bindings) override;
  void bindIndexBuffer(const BufferBinding &binding,
                       IndexElementSize indexElementSize) override;
  void bindFragmentSamplers(UInt32 slot,
                            const Array<TextureSamplerBinding> &bindings) override;
  void bindFragmentSampler(UInt32 startSlot,
                           const TextureSamplerBinding &binding) override;
  void setViewport(const Viewport &viewport) override;
  void setScissor(Int32 x, Int32 y, Int32 width, Int32 height) override;
  void drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                      UInt32 firstVertex, UInt32 firstInstance) override;
  void drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                             UInt32 firstIndex, UInt32 vertexOffset,
                             UInt32 firstInstance) override;

private:
  void bindUniforms();

  CommandBuffer *commandBuffer;
  Ptr<GraphicsPipeline> pipeline;
};
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_COMMAND_HPP
