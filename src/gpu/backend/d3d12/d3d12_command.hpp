#ifndef SINEN_D3D12_COMMAND_HPP
#define SINEN_D3D12_COMMAND_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_resources.hpp"

#include <gpu/gpu_command_buffer.hpp>
#include <gpu/gpu_compute_pass.hpp>
#include <gpu/gpu_copy_pass.hpp>
#include <gpu/gpu_render_pass.hpp>

#include <array>
#include <cstddef>
#include <vector>

namespace sinen::gpu::d3d12 {
class Device;

// Device-owned storage has no engine Device reference. It can outlive the
// recording wrapper until its submission fence completes, without a cycle.
struct CommandContext {
  struct UploadPage {
    ComPtr<ID3D12Resource> resource;
    std::byte *mapped = nullptr;
    UINT64 capacity = 0;
    UINT64 used = 0;
  };
  ~CommandContext();
  ComPtr<ID3D12CommandAllocator> allocator;
  ComPtr<ID3D12GraphicsCommandList> list;
  std::vector<UploadPage> pages;
  std::vector<ComPtr<IUnknown>> retainedObjects;
  Size pageIndex = 0;
  UINT64 fenceValue = 0;
  UINT frameIndex = 0;
  bool closed = false;
};

class CommandBuffer : public gpu::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo, Ptr<Device> device,
                const std::shared_ptr<CommandContext> &context);
  ~CommandBuffer() override { close(); }

  ID3D12GraphicsCommandList *getNative() const { return commandList.Get(); }
  Ptr<Device> getDevice() const { return device; }
  void close();
  bool usesSwapchain() const { return swapchainUsed; }
  void markUsesSwapchain() { swapchainUsed = true; }
  const std::shared_ptr<CommandContext> &getContext() const { return context; }
  Ptr<gpu::Buffer> createTransientBuffer(const gpu::Buffer::CreateInfo &info,
                                         const void *data) override;

  Ptr<gpu::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<gpu::CopyPass> copyPass) override;
  Ptr<gpu::ComputePass>
  beginComputePass(const Array<StorageTextureBinding> &storageTextures,
                   const Array<StorageBufferBinding> &storageBuffers) override;
  void endComputePass(Ptr<gpu::ComputePass> computePass) override;
  Ptr<gpu::RayTracingPass> beginRayTracingPass() override;
  void endRayTracingPass(Ptr<gpu::RayTracingPass> rayTracingPass) override;
  Ptr<gpu::RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<gpu::RenderPass> renderPass) override;
  void pushVertexUniformData(UInt32 slot, const void *data,
                             size_t size) override;
  void pushFragmentUniformData(UInt32 slot, const void *data,
                               size_t size) override;
  void pushComputeUniformData(UInt32 slot, const void *data,
                              size_t size) override;

  D3D12_GPU_VIRTUAL_ADDRESS vertexUniform(UInt32 slot) const;
  D3D12_GPU_VIRTUAL_ADDRESS fragmentUniform(UInt32 slot) const;
  D3D12_GPU_VIRTUAL_ADDRESS computeUniform(UInt32 slot) const;
  void keepAlive(ComPtr<ID3D12Resource> resource) { keepAlive(resource.Get()); }
  void keepAlive(IUnknown *resource) {
    if (!resource) {
      return;
    }
    resource->AddRef();
    ComPtr<IUnknown> retained;
    retained.Attach(resource);
    context->retainedObjects.push_back(retained);
  }

private:
  D3D12_GPU_VIRTUAL_ADDRESS uploadUniform(const void *data, size_t size);
  bool uploadData(const void *data, Size size, UINT64 alignment,
                  ComPtr<ID3D12Resource> &resource, UINT64 &offset);

  Ptr<Device> device;
  std::shared_ptr<CommandContext> context;
  ComPtr<ID3D12GraphicsCommandList> commandList;
  bool swapchainUsed = false;
  std::array<D3D12_GPU_VIRTUAL_ADDRESS, 4> vertexUniforms{};
  std::array<D3D12_GPU_VIRTUAL_ADDRESS, 4> fragmentUniforms{};
  std::array<D3D12_GPU_VIRTUAL_ADDRESS, 4> computeUniforms{};
};

class CopyPass : public gpu::CopyPass {
public:
  bool copyBufferToTexture(const BufferRegion &src, const TextureRegion &dst,
                           UInt32 bytesPerRow) override;
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

  void
  bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> graphicsPipeline) override;
  void bindVertexBuffers(UInt32 slot,
                         const Array<BufferBinding> &bindings) override;
  void bindIndexBuffer(const BufferBinding &binding,
                       IndexElementSize indexElementSize) override;
  void
  bindFragmentSamplers(UInt32 slot,
                       const Array<TextureSamplerBinding> &bindings) override;
  void bindFragmentSampler(UInt32 startSlot,
                           const TextureSamplerBinding &binding) override;
  void bindAccelerationStructures(UInt32 startSlot,
                                  const Array<Ptr<gpu::AccelerationStructure>>
                                      &accelerationStructures) override;
  void setViewport(const Viewport &viewport) override;
  void setScissor(Int32 x, Int32 y, Int32 width, Int32 height) override;
  void drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                      UInt32 firstVertex, UInt32 firstInstance) override;
  void drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                             UInt32 firstIndex, UInt32 vertexOffset,
                             UInt32 firstInstance) override;
  void resolveTargets();

private:
  void bindUniforms();

  CommandBuffer *commandBuffer;
  Array<ColorTargetInfo> colorTargets;
  Ptr<GraphicsPipeline> pipeline;
};

class ComputePass : public gpu::ComputePass {
public:
  ComputePass(CommandBuffer *commandBuffer,
              const Array<StorageBufferBinding> &storageBuffers);

  void bindComputePipeline(Ptr<gpu::ComputePipeline> computePipeline) override;
  void bindAccelerationStructures(UInt32 startSlot,
                                  const Array<Ptr<gpu::AccelerationStructure>>
                                      &accelerationStructures) override;
  void dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                          UInt32 groupCountZ) override;

private:
  void bindStorageBuffers();
  void bindUniforms();

  CommandBuffer *commandBuffer;
  Ptr<ComputePipeline> pipeline;
  Array<StorageBufferBinding> storageBuffers;
};

class RayTracingPass : public gpu::RayTracingPass {
public:
  explicit RayTracingPass(CommandBuffer *commandBuffer)
      : commandBuffer(commandBuffer) {}

  void buildBottomLevelAccelerationStructure(
      Ptr<gpu::AccelerationStructure> dst,
      const Array<gpu::RayTracingGeometry> &geometries,
      Ptr<gpu::Buffer> scratchBuffer, UInt64 scratchOffset,
      gpu::RayTracingBuildFlags flags,
      Ptr<gpu::AccelerationStructure> src = nullptr) override;
  void buildTopLevelAccelerationStructure(
      Ptr<gpu::AccelerationStructure> dst, const gpu::BufferBinding &instances,
      UInt32 instanceCount, Ptr<gpu::Buffer> scratchBuffer,
      UInt64 scratchOffset, gpu::RayTracingBuildFlags flags,
      Ptr<gpu::AccelerationStructure> src = nullptr) override;
  void bindAccelerationStructures(UInt32 startSlot,
                                  const Array<Ptr<gpu::AccelerationStructure>>
                                      &accelerationStructures) override;
  void bindStorageBuffers(
      UInt32 startSlot,
      const Array<gpu::StorageBufferBinding> &storageBuffers) override;
  void bindRayTracingPipeline(Ptr<gpu::RayTracingPipeline> pipeline) override;
  void dispatchRays(const gpu::RayTracingShaderTableRegion &rayGeneration,
                    const gpu::RayTracingShaderTableRegion &miss,
                    const gpu::RayTracingShaderTableRegion &hit,
                    const gpu::RayTracingShaderTableRegion &callable,
                    UInt32 width, UInt32 height, UInt32 depth) override;

private:
  void bindUniforms();

  CommandBuffer *commandBuffer;
  Ptr<RayTracingPipeline> pipeline;
};
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_COMMAND_HPP
