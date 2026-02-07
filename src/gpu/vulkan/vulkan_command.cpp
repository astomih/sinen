#include "vulkan_command.hpp"

#ifndef EMSCRIPTEN
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cstring>

namespace sinen::gpu::vulkan {
static VkDeviceSize alignUp(VkDeviceSize value, VkDeviceSize alignment) {
  if (alignment == 0) {
    return value;
  }
  return (value + alignment - 1) & ~(alignment - 1);
}

static void setBarrierMasksForLayout(VkImageLayout layout,
                                     VkAccessFlags &accessMask) {
  switch (layout) {
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    accessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    accessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    accessMask = 0;
    break;
  default:
    accessMask = 0;
    break;
  }
}

CommandBuffer::CommandBuffer(const CreateInfo &createInfo, Device &device,
                             VkCommandBuffer commandBuffer, VkFence fence,
                             VkDescriptorPool descriptorPool,
                             VkBuffer uniformBuffer,
                             VmaAllocation uniformAllocation,
                             void *uniformMapped, VkDeviceSize uniformSize,
                             VkDeviceSize uniformAlignment)
    : gpu::CommandBuffer(createInfo), device(device),
      commandBuffer(commandBuffer), fence(fence),
      descriptorPool(descriptorPool), uniformBuffer(uniformBuffer),
      uniformAllocation(uniformAllocation), uniformMapped(uniformMapped),
      uniformSize(uniformSize), uniformAlignment(uniformAlignment),
      uniformRange(device.getUniformRange()),
      uniformSlotOffsets(createInfo.allocator) {
  uniformSlotOffsets.resize(32);
  std::fill(uniformSlotOffsets.begin(), uniformSlotOffsets.end(), 0u);
}

CommandBuffer::~CommandBuffer() {
  VkDevice vkDevice = device.getVkDevice();
  if (vkDevice == VK_NULL_HANDLE) {
    return;
  }

  if (descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
  }
  if (uniformBuffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(device.getVmaAllocator(), uniformBuffer,
                     uniformAllocation);
  }
  if (fence != VK_NULL_HANDLE) {
    vkDestroyFence(vkDevice, fence, nullptr);
  }
  if (commandBuffer != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(vkDevice, device.getCommandPool(), 1, &commandBuffer);
  }
}

void CommandBuffer::markUsesSwapchain(uint32_t imageIndex) {
  swapchainUsed = true;
  swapchainImageIndex = imageIndex;
}

void CommandBuffer::setUniformSlotOffset(uint32_t slot, uint32_t offset) {
  if (slot >= static_cast<uint32_t>(uniformSlotOffsets.size())) {
    uniformSlotOffsets.resize(slot + 1);
  }
  uniformSlotOffsets[slot] = offset;
}

uint32_t CommandBuffer::getUniformSlotOffset(uint32_t slot) const {
  if (slot >= static_cast<uint32_t>(uniformSlotOffsets.size())) {
    return 0;
  }
  return uniformSlotOffsets[slot];
}

void CommandBuffer::ensureRecording() {
  if (recording) {
    return;
  }

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkBeginCommandBuffer failed");
    return;
  }
  recording = true;
  uniformWriteOffset = 0;
  std::fill(uniformSlotOffsets.begin(), uniformSlotOffsets.end(), 0u);
}

void CommandBuffer::transitionTexture(Texture &texture,
                                      VkImageLayout newLayout) {
  if (texture.getLayout() == newLayout) {
    return;
  }

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = texture.getLayout();
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = texture.getImage();
  barrier.subresourceRange.aspectMask = texture.getAspectMask();
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = texture.getMipLevels();
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = texture.getLayers();

  setBarrierMasksForLayout(barrier.oldLayout, barrier.srcAccessMask);
  setBarrierMasksForLayout(barrier.newLayout, barrier.dstAccessMask);

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                       VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  texture.setLayout(newLayout);
}

void CommandBuffer::finishForSubmit() {
  if (!recording) {
    return;
  }

  if (swapchainUsed) {
    Texture *swapTex = device.getSwapchainTexture(swapchainImageIndex);
    if (swapTex) {
      transitionTexture(*swapTex, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
  }

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkEndCommandBuffer failed");
  }
  recording = false;
}

Ptr<gpu::CopyPass> CommandBuffer::beginCopyPass() {
  ensureRecording();
  return makePtr<CopyPass>(getCreateInfo().allocator, device, *this);
}

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass> /*copyPass*/) {}

Ptr<gpu::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  ensureRecording();
  auto pass = makePtr<RenderPass>(getCreateInfo().allocator, device, *this);
  pass->begin(infos, depthStencilInfo, r, g, b, a);
  return pass;
}

void CommandBuffer::endRenderPass(Ptr<gpu::RenderPass> renderPass) {
  downCast<RenderPass>(renderPass)->end();
}

void CommandBuffer::pushUniformData(UInt32 slot, const void *data, Size size) {
  ensureRecording();
  if (!uniformMapped) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform buffer not mapped");
    return;
  }
  if (size > uniformRange) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform push too large (%zu > %llu)", size,
                 static_cast<unsigned long long>(uniformRange));
    return;
  }

  VkDeviceSize aligned = alignUp(uniformWriteOffset, uniformAlignment);
  if (aligned + size > uniformSize) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform buffer out of space");
    return;
  }
  std::memcpy(static_cast<uint8_t *>(uniformMapped) + aligned, data, size);
  vmaFlushAllocation(device.getVmaAllocator(), uniformAllocation, aligned,
                     size);
  setUniformSlotOffset(slot, static_cast<uint32_t>(aligned));
  uniformWriteOffset = aligned + size;
}

CopyPass::CopyPass(Device &device, CommandBuffer &commandBuffer)
    : device(device), commandBuffer(commandBuffer) {}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool /*cycle*/) {
  auto srcBuf = downCast<TransferBuffer>(src.transferBuffer);
  auto dstBuf = downCast<Buffer>(dst.buffer);
  VkBufferCopy region{};
  region.srcOffset = src.offset;
  region.dstOffset = dst.offset;
  region.size = dst.size;
  vkCmdCopyBuffer(commandBuffer.getNative(), srcBuf->getNative(),
                  dstBuf->getNative(), 1, &region);
}

void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  auto srcBuf = downCast<Buffer>(src.buffer);
  auto dstBuf = downCast<TransferBuffer>(dst.transferBuffer);
  VkBufferCopy region{};
  region.srcOffset = src.offset;
  region.dstOffset = dst.offset;
  region.size = src.size;
  vkCmdCopyBuffer(commandBuffer.getNative(), srcBuf->getNative(),
                  dstBuf->getNative(), 1, &region);
}

void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool /*cycle*/) {
  auto srcBuf = downCast<TransferBuffer>(src.transferBuffer);
  auto dstTex = downCast<Texture>(dst.texture);

  commandBuffer.transitionTexture(*dstTex,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkBufferImageCopy copy{};
  copy.bufferOffset = src.offset;
  copy.imageSubresource.aspectMask = dstTex->getAspectMask();
  copy.imageSubresource.mipLevel = dst.mipLevel;
  copy.imageSubresource.baseArrayLayer = dst.layer;
  copy.imageSubresource.layerCount = 1;
  copy.imageOffset = {static_cast<int32_t>(dst.x), static_cast<int32_t>(dst.y),
                      static_cast<int32_t>(dst.z)};
  copy.imageExtent = {dst.width, dst.height, dst.depth};
  vkCmdCopyBufferToImage(commandBuffer.getNative(), srcBuf->getNative(),
                         dstTex->getImage(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

  if (dstTex->getCreateInfo().usage == gpu::TextureUsage::Sampler) {
    commandBuffer.transitionTexture(*dstTex,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
}

void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {
  auto srcTex = downCast<Texture>(src.texture);
  auto dstBuf = downCast<TransferBuffer>(dst.transferBuffer);

  commandBuffer.transitionTexture(*srcTex,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  VkBufferImageCopy copy{};
  copy.bufferOffset = dst.offset;
  copy.imageSubresource.aspectMask = srcTex->getAspectMask();
  copy.imageSubresource.mipLevel = src.mipLevel;
  copy.imageSubresource.baseArrayLayer = src.layer;
  copy.imageSubresource.layerCount = 1;
  copy.imageOffset = {static_cast<int32_t>(src.x), static_cast<int32_t>(src.y),
                      static_cast<int32_t>(src.z)};
  copy.imageExtent = {src.width, src.height, src.depth};
  vkCmdCopyImageToBuffer(commandBuffer.getNative(), srcTex->getImage(),
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         dstBuf->getNative(), 1, &copy);
}

void CopyPass::copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, UInt32 width,
                           UInt32 height, UInt32 depth, bool /*cycle*/) {
  auto srcTex = downCast<Texture>(src.texture);
  auto dstTex = downCast<Texture>(dst.texture);
  commandBuffer.transitionTexture(*srcTex,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  commandBuffer.transitionTexture(*dstTex,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkImageCopy copy{};
  copy.srcSubresource.aspectMask = srcTex->getAspectMask();
  copy.srcSubresource.mipLevel = src.mipLevel;
  copy.srcSubresource.baseArrayLayer = src.layer;
  copy.srcSubresource.layerCount = 1;
  copy.srcOffset = {static_cast<int32_t>(src.x), static_cast<int32_t>(src.y),
                    static_cast<int32_t>(src.z)};
  copy.dstSubresource.aspectMask = dstTex->getAspectMask();
  copy.dstSubresource.mipLevel = dst.mipLevel;
  copy.dstSubresource.baseArrayLayer = dst.layer;
  copy.dstSubresource.layerCount = 1;
  copy.dstOffset = {static_cast<int32_t>(dst.x), static_cast<int32_t>(dst.y),
                    static_cast<int32_t>(dst.z)};
  copy.extent = {width, height, depth};
  vkCmdCopyImage(commandBuffer.getNative(), srcTex->getImage(),
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstTex->getImage(),
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

RenderPass::RenderPass(Device &device, CommandBuffer &commandBuffer)
    : device(device), commandBuffer(commandBuffer),
      cmd(commandBuffer.getNative()) {}

void RenderPass::begin(const Array<ColorTargetInfo> &infos,
                       const DepthStencilTargetInfo &depthStencilInfo, float r,
                       float g, float b, float a) {
  Array<VkRenderingAttachmentInfo> colorAttachments(
      commandBuffer.getCreateInfo().allocator);
  colorAttachments.resize(infos.size());

  uint32_t width = 1;
  uint32_t height = 1;

  for (int i = 0; i < infos.size(); ++i) {
    auto tex = downCast<Texture>(infos[i].texture);
    commandBuffer.transitionTexture(*tex,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    width = tex->getWidth();
    height = tex->getHeight();

    VkRenderingAttachmentInfo attachment{};
    attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    attachment.imageView = tex->getView();
    attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp = convert::loadOpFrom(infos[i].loadOp);
    attachment.storeOp = convert::storeOpFrom(infos[i].storeOp);
    attachment.clearValue.color = {{r, g, b, a}};
    colorAttachments[i] = attachment;
  }

  VkRenderingAttachmentInfo depthAttachment{};
  VkRenderingAttachmentInfo stencilAttachment{};
  VkRenderingInfo rendering{};
  rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering.renderArea.offset = {0, 0};
  rendering.renderArea.extent = {width, height};
  rendering.layerCount = 1;
  rendering.colorAttachmentCount = colorAttachments.size();
  rendering.pColorAttachments = colorAttachments.data();

  if (depthStencilInfo.texture) {
    auto tex = downCast<Texture>(depthStencilInfo.texture);
    commandBuffer.transitionTexture(
        *tex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = tex->getView();
    depthAttachment.imageLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = convert::loadOpFrom(depthStencilInfo.loadOp);
    depthAttachment.storeOp = convert::storeOpFrom(depthStencilInfo.storeOp);
    depthAttachment.clearValue.depthStencil.depth = depthStencilInfo.clearDepth;
    depthAttachment.clearValue.depthStencil.stencil =
        static_cast<uint32_t>(depthStencilInfo.clearStencil);
    rendering.pDepthAttachment = &depthAttachment;

    stencilAttachment = depthAttachment;
    stencilAttachment.loadOp =
        convert::loadOpFrom(depthStencilInfo.stencilLoadOp);
    stencilAttachment.storeOp =
        convert::storeOpFrom(depthStencilInfo.stencilStoreOp);
    rendering.pStencilAttachment = &stencilAttachment;
  }

  vkCmdBeginRendering(cmd, &rendering);
}

void RenderPass::end() { vkCmdEndRendering(cmd); }

void RenderPass::ensureDescriptorSet() {
  if (!boundPipeline || uniformSet != VK_NULL_HANDLE ||
      samplerSet != VK_NULL_HANDLE) {
    return;
  }
  auto layoutInfo = boundPipeline->getLayoutInfo();

  std::array<VkDescriptorSetLayout, 2> setLayouts = {
      layoutInfo.uniformSetLayout, layoutInfo.samplerSetLayout};

  std::array<VkDescriptorSet, 2> sets = {VK_NULL_HANDLE, VK_NULL_HANDLE};

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = commandBuffer.getDescriptorPool();
  allocInfo.descriptorSetCount = static_cast<uint32_t>(setLayouts.size());
  allocInfo.pSetLayouts = setLayouts.data();

  if (vkAllocateDescriptorSets(device.getVkDevice(), &allocInfo, sets.data()) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkAllocateDescriptorSets failed");
    return;
  }
  uniformSet = sets[0];
  samplerSet = sets[1];

  Array<VkWriteDescriptorSet> writes(commandBuffer.getCreateInfo().allocator);
  writes.reserve(layoutInfo.uniformBindingCount +
                 layoutInfo.samplerBindingCount);
  Array<VkDescriptorBufferInfo> bufferInfos(
      commandBuffer.getCreateInfo().allocator);
  bufferInfos.resize(layoutInfo.uniformBindingCount);

  for (uint32_t i = 0; i < layoutInfo.uniformBindingCount; ++i) {
    VkDescriptorBufferInfo bi{};
    bi.buffer = commandBuffer.getUniformBuffer();
    bi.offset = 0;
    bi.range = commandBuffer.getUniformRange();
    bufferInfos[i] = bi;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = uniformSet;
    w.dstBinding = i;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    w.pBufferInfo = &bufferInfos[i];
    writes.push_back(w);
  }

  Array<VkDescriptorImageInfo> imageInfos(
      commandBuffer.getCreateInfo().allocator);
  imageInfos.resize(layoutInfo.samplerBindingCount);
  for (uint32_t i = 0; i < layoutInfo.samplerBindingCount; ++i) {
    VkDescriptorImageInfo ii{};
    ii.sampler = device.getDefaultSampler();
    ii.imageView = device.getDefaultTextureView();
    ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[i] = ii;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = samplerSet;
    w.dstBinding = i;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    w.pImageInfo = &imageInfos[i];
    writes.push_back(w);
  }

  vkUpdateDescriptorSets(device.getVkDevice(), writes.size(), writes.data(), 0,
                         nullptr);
}

void RenderPass::bindDescriptorSet() {
  if (!boundPipeline || uniformSet == VK_NULL_HANDLE ||
      samplerSet == VK_NULL_HANDLE) {
    return;
  }

  auto layoutInfo = boundPipeline->getLayoutInfo();
  Array<uint32_t> dynamicOffsets(commandBuffer.getCreateInfo().allocator);
  dynamicOffsets.resize(layoutInfo.uniformBindingCount);
  for (uint32_t i = 0; i < layoutInfo.uniformBindingCount; ++i) {
    dynamicOffsets[i] = commandBuffer.getUniformSlotOffset(i);
  }
  std::array<VkDescriptorSet, 2> sets = {uniformSet, samplerSet};
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          layoutInfo.pipelineLayout, 1,
                          static_cast<uint32_t>(sets.size()), sets.data(),
                          dynamicOffsets.size(), dynamicOffsets.data());
}

void RenderPass::bindGraphicsPipeline(
    Ptr<gpu::GraphicsPipeline> graphicsPipeline) {
  boundPipeline = downCast<GraphicsPipeline>(graphicsPipeline);
  uniformSet = VK_NULL_HANDLE;
  samplerSet = VK_NULL_HANDLE;
  ensureDescriptorSet();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    boundPipeline->getNative());
}

void RenderPass::bindVertexBuffers(UInt32 startSlot,
                                   const Array<BufferBinding> &bindings) {
  Array<VkBuffer> buffers(commandBuffer.getCreateInfo().allocator);
  Array<VkDeviceSize> offsets(commandBuffer.getCreateInfo().allocator);
  buffers.resize(bindings.size());
  offsets.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    buffers[i] = downCast<Buffer>(bindings[i].buffer)->getNative();
    offsets[i] = bindings[i].offset;
  }
  vkCmdBindVertexBuffers(cmd, startSlot, buffers.size(), buffers.data(),
                         offsets.data());
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  VkIndexType indexType = VK_INDEX_TYPE_UINT32;
  if (indexElementSize == IndexElementSize::Uint16) {
    indexType = VK_INDEX_TYPE_UINT16;
  }
  vkCmdBindIndexBuffer(cmd, downCast<Buffer>(binding.buffer)->getNative(),
                       binding.offset, indexType);
}

void RenderPass::bindFragmentSamplers(
    UInt32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  if (!boundPipeline) {
    return;
  }
  ensureDescriptorSet();
  Array<VkWriteDescriptorSet> writes(commandBuffer.getCreateInfo().allocator);
  Array<VkDescriptorImageInfo> infos(commandBuffer.getCreateInfo().allocator);
  writes.resize(bindings.size());
  infos.resize(bindings.size());

  for (int i = 0; i < bindings.size(); ++i) {
    auto sampler = downCast<Sampler>(bindings[i].sampler);
    auto texture = downCast<Texture>(bindings[i].texture);
    commandBuffer.transitionTexture(*texture,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo ii{};
    ii.sampler = sampler->getNative();
    ii.imageView = texture->getView();
    ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    infos[i] = ii;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = samplerSet;
    w.dstBinding = startSlot + i;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    w.pImageInfo = &infos[i];
    writes[i] = w;
  }

  vkUpdateDescriptorSets(device.getVkDevice(), writes.size(), writes.data(), 0,
                         nullptr);
}

void RenderPass::bindFragmentSampler(UInt32 startSlot,
                                     const TextureSamplerBinding &binding) {
  Array<TextureSamplerBinding> bindings(
      commandBuffer.getCreateInfo().allocator);
  bindings.push_back(binding);
  bindFragmentSamplers(startSlot, bindings);
}

void RenderPass::setViewport(const Viewport &viewport) {
  VkViewport vp{};
  vp.x = viewport.x;
  vp.y = viewport.y;
  vp.width = viewport.width;
  vp.height = viewport.height;
  vp.minDepth = viewport.minDepth;
  vp.maxDepth = viewport.maxDepth;
  vkCmdSetViewport(cmd, 0, 1, &vp);
}

void RenderPass::setScissor(Int32 x, Int32 y, Int32 width, Int32 height) {
  VkRect2D scissor{};
  scissor.offset = {x, y};
  scissor.extent = {static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)};
  vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void RenderPass::drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                                UInt32 firstVertex, UInt32 firstInstance) {
  ensureDescriptorSet();
  bindDescriptorSet();
  vkCmdDraw(cmd, numVertices, numInstances, firstVertex, firstInstance);
}

void RenderPass::drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                                       UInt32 firstIndex, UInt32 vertexOffset,
                                       UInt32 firstInstance) {
  ensureDescriptorSet();
  bindDescriptorSet();
  vkCmdDrawIndexed(cmd, numIndices, numInstances, firstIndex, vertexOffset,
                   firstInstance);
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
