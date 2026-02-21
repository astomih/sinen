#pragma once
#ifndef EMSCRIPTEN

#include <core/data/array.hpp>
#include <gpu/gpu_command_buffer.hpp>

#include "vulkan_resources.hpp"

namespace sinen::gpu::vulkan {
class Device;

class CommandBuffer : public gpu::CommandBuffer {
public:
  CommandBuffer(const CreateInfo &createInfo, Device &device,
                VkCommandBuffer commandBuffer, VkFence fence,
                VkDescriptorPool descriptorPool, VkBuffer uniformBuffer,
                VmaAllocation uniformAllocation, void *uniformMapped,
                VkDeviceSize uniformSize, VkDeviceSize uniformAlignment);
  ~CommandBuffer() override;

  VkCommandBuffer getNative() const { return commandBuffer; }
  VkFence getFence() const { return fence; }
  VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
  VkBuffer getUniformBuffer() const { return uniformBuffer; }
  VmaAllocation getUniformAllocation() const { return uniformAllocation; }
  VkDeviceSize getUniformRange() const { return uniformRange; }

  void markUsesSwapchain(uint32_t imageIndex);
  bool usesSwapchain() const { return swapchainUsed; }
  uint32_t getSwapchainImageIndex() const { return swapchainImageIndex; }

  void ensureRecording();
  void finishForSubmit();

  void transitionTexture(Texture &texture, VkImageLayout newLayout);

  void setUniformSlotOffset(uint32_t slot, uint32_t offset);
  uint32_t getUniformSlotOffset(uint32_t slot) const;

  Ptr<gpu::CopyPass> beginCopyPass() override;
  void endCopyPass(Ptr<gpu::CopyPass> copyPass) override;
  Ptr<gpu::RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) override;
  void endRenderPass(Ptr<gpu::RenderPass> renderPass) override;
  void pushVertexUniformData(UInt32 slot, const void *data, Size size) override;
  void pushFragmentUniformData(UInt32 slot, const void *data,
                               Size size) override;

private:
  void pushUniformDataInternal(UInt32 slot, const void *data, Size size);

  Device &device;
  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

  VkBuffer uniformBuffer = VK_NULL_HANDLE;
  VmaAllocation uniformAllocation = VK_NULL_HANDLE;
  void *uniformMapped = nullptr;
  VkDeviceSize uniformSize = 0;
  VkDeviceSize uniformAlignment = 256;
  VkDeviceSize uniformWriteOffset = 0;
  VkDeviceSize uniformRange = 0;

  Array<uint32_t> uniformSlotOffsets;

  bool recording = false;
  bool swapchainUsed = false;
  uint32_t swapchainImageIndex = 0;
};

class CopyPass : public gpu::CopyPass {
public:
  CopyPass(Device &device, CommandBuffer &commandBuffer);
  ~CopyPass() override = default;

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
  Device &device;
  CommandBuffer &commandBuffer;
};

class RenderPass : public gpu::RenderPass {
public:
  RenderPass(Device &device, CommandBuffer &commandBuffer);
  ~RenderPass() override = default;

  void begin(const Array<ColorTargetInfo> &infos,
             const DepthStencilTargetInfo &depthStencilInfo, float r, float g,
             float b, float a);
  void end();

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
  void setViewport(const Viewport &viewport) override;
  void setScissor(Int32 x, Int32 y, Int32 width, Int32 height) override;
  void drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                      UInt32 firstVertex, UInt32 firstInstance) override;
  void drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                             UInt32 firstIndex, UInt32 vertexOffset,
                             UInt32 firstInstance) override;

private:
  void ensureDescriptorSet();
  void bindDescriptorSet();

  Device &device;
  CommandBuffer &commandBuffer;
  VkCommandBuffer cmd = VK_NULL_HANDLE;

  Ptr<GraphicsPipeline> boundPipeline = nullptr;
  VkDescriptorSet uniformSet = VK_NULL_HANDLE;
  VkDescriptorSet samplerSet = VK_NULL_HANDLE;
};

} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
