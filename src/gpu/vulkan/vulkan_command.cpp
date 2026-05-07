#include "vulkan_command.hpp"

#ifndef EMSCRIPTEN
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

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
      vertexUniformSlotOffsets(createInfo.allocator),
      fragmentUniformSlotOffsets(createInfo.allocator),
      referencedBuffers(createInfo.allocator),
      referencedTextures(createInfo.allocator),
      referencedSamplers(createInfo.allocator),
      referencedPipelines(createInfo.allocator),
      referencedTransferBuffers(createInfo.allocator),
      referencedRenderPasses(createInfo.allocator),
      referencedFramebuffers(createInfo.allocator) {
  vertexUniformSlotOffsets.resize(32);
  fragmentUniformSlotOffsets.resize(32);
  std::fill(vertexUniformSlotOffsets.begin(), vertexUniformSlotOffsets.end(),
            0u);
  std::fill(fragmentUniformSlotOffsets.begin(),
            fragmentUniformSlotOffsets.end(), 0u);
}

CommandBuffer::~CommandBuffer() {
  VkDevice vkDevice = device.getVkDevice();
  if (vkDevice == VK_NULL_HANDLE) {
    return;
  }

  if (descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
  }
  for (VkFramebuffer framebuffer : referencedFramebuffers) {
    vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
  }
  for (VkRenderPass renderPass : referencedRenderPasses) {
    vkDestroyRenderPass(vkDevice, renderPass, nullptr);
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

void CommandBuffer::keepAlive(Ptr<gpu::Buffer> resource) {
  if (resource) {
    referencedBuffers.push_back(resource);
  }
}

void CommandBuffer::keepAlive(Ptr<gpu::Texture> resource) {
  if (resource) {
    referencedTextures.push_back(resource);
  }
}

void CommandBuffer::keepAlive(Ptr<gpu::Sampler> resource) {
  if (resource) {
    referencedSamplers.push_back(resource);
  }
}

void CommandBuffer::keepAlive(Ptr<gpu::GraphicsPipeline> resource) {
  if (resource) {
    referencedPipelines.push_back(resource);
  }
}

void CommandBuffer::keepAlive(Ptr<gpu::TransferBuffer> resource) {
  if (resource) {
    referencedTransferBuffers.push_back(resource);
  }
}

void CommandBuffer::keepAliveRenderPassHandles(VkRenderPass renderPass,
                                               VkFramebuffer framebuffer) {
  if (framebuffer != VK_NULL_HANDLE) {
    referencedFramebuffers.push_back(framebuffer);
  }
  if (renderPass != VK_NULL_HANDLE) {
    referencedRenderPasses.push_back(renderPass);
  }
}

void CommandBuffer::setVertexUniformSlotOffset(uint32_t slot, uint32_t offset) {
  if (slot >= static_cast<uint32_t>(vertexUniformSlotOffsets.size())) {
    vertexUniformSlotOffsets.resize(slot + 1);
  }
  vertexUniformSlotOffsets[slot] = offset;
}

void CommandBuffer::setFragmentUniformSlotOffset(uint32_t slot,
                                                 uint32_t offset) {
  if (slot >= static_cast<uint32_t>(fragmentUniformSlotOffsets.size())) {
    fragmentUniformSlotOffsets.resize(slot + 1);
  }
  fragmentUniformSlotOffsets[slot] = offset;
}

uint32_t CommandBuffer::getVertexUniformSlotOffset(uint32_t slot) const {
  if (slot >= static_cast<uint32_t>(vertexUniformSlotOffsets.size())) {
    return 0;
  }
  return vertexUniformSlotOffsets[slot];
}

uint32_t CommandBuffer::getFragmentUniformSlotOffset(uint32_t slot) const {
  if (slot >= static_cast<uint32_t>(fragmentUniformSlotOffsets.size())) {
    return 0;
  }
  return fragmentUniformSlotOffsets[slot];
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
  std::fill(vertexUniformSlotOffsets.begin(), vertexUniformSlotOffsets.end(),
            0u);
  std::fill(fragmentUniformSlotOffsets.begin(),
            fragmentUniformSlotOffsets.end(), 0u);
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

void CommandBuffer::pushVertexUniformData(UInt32 slot, const void *data,
                                          Size size) {
  setVertexUniformSlotOffset(slot, pushUniformDataInternal(data, size));
}

void CommandBuffer::pushFragmentUniformData(UInt32 slot, const void *data,
                                            Size size) {
  setFragmentUniformSlotOffset(slot, pushUniformDataInternal(data, size));
}

uint32_t CommandBuffer::pushUniformDataInternal(const void *data, Size size) {
  ensureRecording();
  if (!uniformMapped) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform buffer not mapped");
    return 0;
  }
  if (size > uniformRange) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform push too large (%zu > %llu)", size,
                 static_cast<unsigned long long>(uniformRange));
    return 0;
  }

  VkDeviceSize aligned = alignUp(uniformWriteOffset, uniformAlignment);
  if (aligned + size > uniformSize) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: uniform buffer out of space");
    return 0;
  }
  std::memcpy(static_cast<uint8_t *>(uniformMapped) + aligned, data, size);
  vmaFlushAllocation(device.getVmaAllocator(), uniformAllocation, aligned,
                     size);
  uniformWriteOffset = aligned + size;
  return static_cast<uint32_t>(aligned);
}

CopyPass::CopyPass(Device &device, CommandBuffer &commandBuffer)
    : device(device), commandBuffer(commandBuffer) {}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool /*cycle*/) {
  commandBuffer.keepAlive(src.transferBuffer);
  commandBuffer.keepAlive(dst.buffer);
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
  commandBuffer.keepAlive(src.buffer);
  commandBuffer.keepAlive(dst.transferBuffer);
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
  commandBuffer.keepAlive(src.transferBuffer);
  commandBuffer.keepAlive(dst.texture);
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
  commandBuffer.keepAlive(src.texture);
  commandBuffer.keepAlive(dst.transferBuffer);
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
  commandBuffer.keepAlive(src.texture);
  commandBuffer.keepAlive(dst.texture);
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
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference> colorRefs;
  std::vector<VkImageView> views;
  std::vector<VkClearValue> clearValues;
  attachments.reserve(infos.size() + 1);
  colorRefs.reserve(infos.size());
  views.reserve(infos.size() + 1);
  clearValues.reserve(infos.size() + 1);

  uint32_t width = 1;
  uint32_t height = 1;

  for (const auto &info : infos) {
    commandBuffer.keepAlive(info.texture);
    auto tex = downCast<Texture>(info.texture);
    width = tex->getWidth();
    height = tex->getHeight();

    VkAttachmentDescription attachment{};
    attachment.format = tex->getFormat();
    attachment.samples =
        convert::sampleCountFrom(tex->getCreateInfo().sampleCount);
    attachment.loadOp = convert::loadOpFrom(info.loadOp);
    attachment.storeOp = convert::storeOpFrom(info.storeOp);
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = tex->getLayout();
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments.push_back(attachment);

    VkAttachmentReference ref{};
    ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorRefs.push_back(ref);
    views.push_back(tex->getView());

    VkClearValue clear{};
    clear.color = {{r, g, b, a}};
    clearValues.push_back(clear);
    tex->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }

  VkAttachmentReference depthRef{};
  VkAttachmentReference *depthRefPtr = nullptr;

  if (depthStencilInfo.texture) {
    commandBuffer.keepAlive(depthStencilInfo.texture);
    auto tex = downCast<Texture>(depthStencilInfo.texture);
    VkAttachmentDescription attachment{};
    attachment.format = tex->getFormat();
    attachment.samples =
        convert::sampleCountFrom(tex->getCreateInfo().sampleCount);
    attachment.loadOp = convert::loadOpFrom(depthStencilInfo.loadOp);
    attachment.storeOp = convert::storeOpFrom(depthStencilInfo.storeOp);
    attachment.stencilLoadOp =
        convert::loadOpFrom(depthStencilInfo.stencilLoadOp);
    attachment.stencilStoreOp =
        convert::storeOpFrom(depthStencilInfo.stencilStoreOp);
    attachment.initialLayout = tex->getLayout();
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments.push_back(attachment);

    depthRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthRefPtr = &depthRef;
    views.push_back(tex->getView());

    VkClearValue clear{};
    clear.depthStencil.depth = depthStencilInfo.clearDepth;
    clear.depthStencil.stencil =
        static_cast<uint32_t>(depthStencilInfo.clearStencil);
    clearValues.push_back(clear);
    tex->setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  }

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
  subpass.pColorAttachments = colorRefs.data();
  subpass.pDepthStencilAttachment = depthRefPtr;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = dependency.srcStageMask;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassCI{};
  renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassCI.pAttachments = attachments.data();
  renderPassCI.subpassCount = 1;
  renderPassCI.pSubpasses = &subpass;
  renderPassCI.dependencyCount = 1;
  renderPassCI.pDependencies = &dependency;

  if (vkCreateRenderPass(device.getVkDevice(), &renderPassCI, nullptr,
                         &renderPass) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateRenderPass failed");
    return;
  }

  VkFramebufferCreateInfo framebufferCI{};
  framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCI.renderPass = renderPass;
  framebufferCI.attachmentCount = static_cast<uint32_t>(views.size());
  framebufferCI.pAttachments = views.data();
  framebufferCI.width = width;
  framebufferCI.height = height;
  framebufferCI.layers = 1;
  if (vkCreateFramebuffer(device.getVkDevice(), &framebufferCI, nullptr,
                          &framebuffer) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateFramebuffer failed");
    vkDestroyRenderPass(device.getVkDevice(), renderPass, nullptr);
    renderPass = VK_NULL_HANDLE;
    return;
  }

  VkRenderPassBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.renderPass = renderPass;
  beginInfo.framebuffer = framebuffer;
  beginInfo.renderArea.offset = {0, 0};
  beginInfo.renderArea.extent = {width, height};
  beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  beginInfo.pClearValues = clearValues.data();
  vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end() {
  if (renderPass == VK_NULL_HANDLE) {
    return;
  }
  vkCmdEndRenderPass(cmd);
  commandBuffer.keepAliveRenderPassHandles(renderPass, framebuffer);
  framebuffer = VK_NULL_HANDLE;
  renderPass = VK_NULL_HANDLE;
}

void RenderPass::ensureDescriptorSet() {
  if (!boundPipeline || vertexUniformSet != VK_NULL_HANDLE ||
      fragmentSamplerSet != VK_NULL_HANDLE ||
      fragmentUniformSet != VK_NULL_HANDLE) {
    return;
  }
  auto layoutInfo = boundPipeline->getLayoutInfo();

  std::array<VkDescriptorSetLayout, 3> setLayouts = {
      layoutInfo.vertexUniformSetLayout, layoutInfo.fragmentSamplerSetLayout,
      layoutInfo.fragmentUniformSetLayout};

  std::array<VkDescriptorSet, 3> sets = {VK_NULL_HANDLE, VK_NULL_HANDLE,
                                         VK_NULL_HANDLE};

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
  vertexUniformSet = sets[0];
  fragmentSamplerSet = sets[1];
  fragmentUniformSet = sets[2];

  Array<VkWriteDescriptorSet> writes(commandBuffer.getCreateInfo().allocator);
  writes.reserve(layoutInfo.vertexUniformBindingCount +
                 layoutInfo.fragmentUniformBindingCount +
                 layoutInfo.fragmentSamplerBindingCount);
  Array<VkDescriptorBufferInfo> vertexBufferInfos(
      commandBuffer.getCreateInfo().allocator);
  vertexBufferInfos.resize(layoutInfo.vertexUniformBindingCount);

  for (uint32_t i = 0; i < layoutInfo.vertexUniformBindingCount; ++i) {
    VkDescriptorBufferInfo bi{};
    bi.buffer = commandBuffer.getUniformBuffer();
    bi.offset = 0;
    bi.range = commandBuffer.getUniformRange();
    vertexBufferInfos[i] = bi;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = vertexUniformSet;
    w.dstBinding = i;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    w.pBufferInfo = &vertexBufferInfos[i];
    writes.push_back(w);
  }

  Array<VkDescriptorBufferInfo> fragmentBufferInfos(
      commandBuffer.getCreateInfo().allocator);
  fragmentBufferInfos.resize(layoutInfo.fragmentUniformBindingCount);
  for (uint32_t i = 0; i < layoutInfo.fragmentUniformBindingCount; ++i) {
    VkDescriptorBufferInfo bi{};
    bi.buffer = commandBuffer.getUniformBuffer();
    bi.offset = 0;
    bi.range = commandBuffer.getUniformRange();
    fragmentBufferInfos[i] = bi;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = fragmentUniformSet;
    w.dstBinding = i;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    w.pBufferInfo = &fragmentBufferInfos[i];
    writes.push_back(w);
  }

  Array<VkDescriptorImageInfo> imageInfos(
      commandBuffer.getCreateInfo().allocator);
  imageInfos.resize(layoutInfo.fragmentSamplerBindingCount);
  for (uint32_t i = 0; i < layoutInfo.fragmentSamplerBindingCount; ++i) {
    VkDescriptorImageInfo ii{};
    ii.sampler = device.getDefaultSampler();
    ii.imageView = device.getDefaultTextureView();
    ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfos[i] = ii;

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstSet = fragmentSamplerSet;
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
  if (!boundPipeline || vertexUniformSet == VK_NULL_HANDLE ||
      fragmentSamplerSet == VK_NULL_HANDLE ||
      fragmentUniformSet == VK_NULL_HANDLE) {
    return;
  }

  auto layoutInfo = boundPipeline->getLayoutInfo();
  Array<uint32_t> dynamicOffsets(commandBuffer.getCreateInfo().allocator);
  dynamicOffsets.reserve(layoutInfo.vertexUniformBindingCount +
                         layoutInfo.fragmentUniformBindingCount);
  for (uint32_t i = 0; i < layoutInfo.vertexUniformBindingCount; ++i) {
    dynamicOffsets.push_back(commandBuffer.getVertexUniformSlotOffset(i));
  }
  for (uint32_t i = 0; i < layoutInfo.fragmentUniformBindingCount; ++i) {
    dynamicOffsets.push_back(commandBuffer.getFragmentUniformSlotOffset(i));
  }
  std::array<VkDescriptorSet, 3> sets = {vertexUniformSet, fragmentSamplerSet,
                                         fragmentUniformSet};
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          layoutInfo.pipelineLayout, 1,
                          static_cast<uint32_t>(sets.size()), sets.data(),
                          dynamicOffsets.size(), dynamicOffsets.data());
}

void RenderPass::bindGraphicsPipeline(
    Ptr<gpu::GraphicsPipeline> graphicsPipeline) {
  commandBuffer.keepAlive(graphicsPipeline);
  boundPipeline = downCast<GraphicsPipeline>(graphicsPipeline);
  if (!boundPipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: null graphics pipeline binding");
    return;
  }
  vertexUniformSet = VK_NULL_HANDLE;
  fragmentSamplerSet = VK_NULL_HANDLE;
  fragmentUniformSet = VK_NULL_HANDLE;
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
    commandBuffer.keepAlive(bindings[i].buffer);
    buffers[i] = downCast<Buffer>(bindings[i].buffer)->getNative();
    offsets[i] = bindings[i].offset;
  }
  vkCmdBindVertexBuffers(cmd, startSlot, buffers.size(), buffers.data(),
                         offsets.data());
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  commandBuffer.keepAlive(binding.buffer);
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
  const auto &layoutInfo = boundPipeline->getLayoutInfo();
  if (startSlot >= layoutInfo.fragmentSamplerBindingCount) {
    return;
  }
  const uint32_t bindingCount = std::min<uint32_t>(
      static_cast<uint32_t>(bindings.size()),
      layoutInfo.fragmentSamplerBindingCount - startSlot);
  Array<VkWriteDescriptorSet> writes(commandBuffer.getCreateInfo().allocator);
  Array<VkDescriptorImageInfo> infos(commandBuffer.getCreateInfo().allocator);
  writes.resize(bindingCount);
  infos.resize(bindingCount);

  for (uint32_t i = 0; i < bindingCount; ++i) {
    commandBuffer.keepAlive(bindings[i].sampler);
    commandBuffer.keepAlive(bindings[i].texture);
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
    w.dstSet = fragmentSamplerSet;
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
  // Match SDL_GPU's Vulkan backend: flip the Vulkan viewport so the public
  // coordinate system stays consistent with D3D12/SDL_GPU.
  vp.y = viewport.y + viewport.height;
  vp.width = viewport.width;
  vp.height = -viewport.height;
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
