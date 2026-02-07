#ifndef EMSCRIPTEN
#include "vulkan_resources.hpp"
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"

namespace sinen::gpu::vulkan {
Buffer::Buffer(const CreateInfo &createInfo, Device &device, VkBuffer buffer,
               VmaAllocation allocation)
    : gpu::Buffer(createInfo), device(device), buffer(buffer),
      allocation(allocation) {}

Buffer::~Buffer() {
  if (buffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(device.getVmaAllocator(), buffer, allocation);
  }
}

TransferBuffer::TransferBuffer(const CreateInfo &createInfo, Device &device,
                               VkBuffer buffer, VmaAllocation allocation,
                               void *mapped)
    : gpu::TransferBuffer(createInfo), device(device), buffer(buffer),
      allocation(allocation), mapped(mapped) {}

TransferBuffer::~TransferBuffer() {
  if (buffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(device.getVmaAllocator(), buffer, allocation);
  }
}

void *TransferBuffer::map(bool /*cycle*/) {
  if (mapped) {
    if (getCreateInfo().usage == gpu::TransferBufferUsage::Download) {
      vmaInvalidateAllocation(device.getVmaAllocator(), allocation, 0,
                              VK_WHOLE_SIZE);
    }
    mappedNow = true;
    return mapped;
  }
  void *out = nullptr;
  if (vmaMapMemory(device.getVmaAllocator(), allocation, &out) != VK_SUCCESS) {
    return nullptr;
  }
  mapped = out;
  if (getCreateInfo().usage == gpu::TransferBufferUsage::Download) {
    vmaInvalidateAllocation(device.getVmaAllocator(), allocation, 0,
                            VK_WHOLE_SIZE);
  }
  mappedNow = true;
  return mapped;
}

void TransferBuffer::unmap() {
  if (!mappedNow) {
    return;
  }
  if (!mapped) {
    return;
  }
  if (getCreateInfo().usage == gpu::TransferBufferUsage::Upload) {
    vmaFlushAllocation(device.getVmaAllocator(), allocation, 0, VK_WHOLE_SIZE);
  }
  vmaUnmapMemory(device.getVmaAllocator(), allocation);
  mapped = nullptr;
  mappedNow = false;
}

Texture::Texture(const CreateInfo &createInfo, Device &device,
                 const NativeCreateInfo &native)
    : gpu::Texture(createInfo), device(device), image(native.image),
      view(native.view), format(native.format), layout(native.initialLayout),
      ownsImage(native.ownsImage), ownsView(native.ownsView),
      allocation(native.allocation), width(native.width), height(native.height),
      depth(native.depth), layers(native.layers), mipLevels(native.mipLevels) {}

Texture::~Texture() {
  if (ownsView && view != VK_NULL_HANDLE) {
    vkDestroyImageView(device.getVkDevice(), view, nullptr);
  }
  if (ownsImage && image != VK_NULL_HANDLE) {
    vmaDestroyImage(device.getVmaAllocator(), image, allocation);
  }
}

VkImageAspectFlags Texture::getAspectMask() const {
  return convert::aspectMaskFrom(format);
}

Sampler::Sampler(const CreateInfo &createInfo, Device &device,
                 VkSampler sampler)
    : gpu::Sampler(createInfo), device(device), sampler(sampler) {}

Sampler::~Sampler() {
  if (sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device.getVkDevice(), sampler, nullptr);
  }
}

Shader::Shader(const CreateInfo &createInfo, Device &device,
               VkShaderModule module)
    : gpu::Shader(createInfo), device(device), module(module),
      entrypoint(createInfo.entrypoint ? createInfo.entrypoint : "main"),
      stage(createInfo.stage), numUniformBuffers(createInfo.numUniformBuffers),
      numSamplers(createInfo.numSamplers) {}

Shader::~Shader() {
  if (module != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device.getVkDevice(), module, nullptr);
  }
}

GraphicsPipeline::GraphicsPipeline(const CreateInfo &createInfo, Device &device,
                                   VkPipeline pipeline,
                                   const LayoutInfo &layoutInfo)
    : gpu::GraphicsPipeline(createInfo), device(device), pipeline(pipeline),
      layoutInfo(layoutInfo) {}

GraphicsPipeline::~GraphicsPipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device.getVkDevice(), pipeline, nullptr);
  }
  if (layoutInfo.pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device.getVkDevice(), layoutInfo.pipelineLayout,
                            nullptr);
  }
  if (layoutInfo.samplerSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.samplerSetLayout, nullptr);
  }
  if (layoutInfo.uniformSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.uniformSetLayout, nullptr);
  }
  if (layoutInfo.emptySetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.emptySetLayout, nullptr);
  }
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
