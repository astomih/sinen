#ifndef EMSCRIPTEN
#include "vulkan_resources.hpp"
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"

namespace sinen::gpu::vulkan {
Buffer::Buffer(const CreateInfo &createInfo, Device &device, VkBuffer buffer,
               VmaAllocation allocation, VkDeviceAddress deviceAddress)
    : gpu::Buffer(createInfo), device(device), buffer(buffer),
      allocation(allocation), deviceAddress(deviceAddress) {}

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
      numSamplers(createInfo.numSamplers),
      numStorageBuffers(createInfo.numStorageBuffers),
      numStorageTextures(createInfo.numStorageTextures) {}

Shader::~Shader() {
  if (module != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device.getVkDevice(), module, nullptr);
  }
}

GraphicsPipeline::GraphicsPipeline(const CreateInfo &createInfo, Device &device,
                                   VkPipeline pipeline, VkRenderPass renderPass,
                                   const LayoutInfo &layoutInfo)
    : gpu::GraphicsPipeline(createInfo), device(device), pipeline(pipeline),
      renderPass(renderPass), layoutInfo(layoutInfo) {}

GraphicsPipeline::~GraphicsPipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device.getVkDevice(), pipeline, nullptr);
  }
  if (renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device.getVkDevice(), renderPass, nullptr);
  }
  if (layoutInfo.pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device.getVkDevice(), layoutInfo.pipelineLayout,
                            nullptr);
  }
  if (layoutInfo.fragmentUniformSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.fragmentUniformSetLayout, nullptr);
  }
  if (layoutInfo.fragmentSamplerSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.fragmentSamplerSetLayout, nullptr);
  }
  if (layoutInfo.vertexUniformSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.vertexUniformSetLayout, nullptr);
  }
  if (layoutInfo.vertexSamplerSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.vertexSamplerSetLayout, nullptr);
  }
}

ComputePipeline::ComputePipeline(const CreateInfo &createInfo, Device &device,
                                 VkPipeline pipeline,
                                 const LayoutInfo &layoutInfo)
    : gpu::ComputePipeline(createInfo), device(device), pipeline(pipeline),
      layoutInfo(layoutInfo) {}

ComputePipeline::~ComputePipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device.getVkDevice(), pipeline, nullptr);
  }
  if (layoutInfo.pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device.getVkDevice(), layoutInfo.pipelineLayout,
                            nullptr);
  }
  if (layoutInfo.uniformSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.uniformSetLayout, nullptr);
  }
  if (layoutInfo.storageBufferSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.storageBufferSetLayout, nullptr);
  }
}

AccelerationStructure::AccelerationStructure(
    const CreateInfo &createInfo, Device &device, VkBuffer buffer,
    VmaAllocation allocation, VkAccelerationStructureKHR accelerationStructure,
    VkDeviceAddress deviceAddress)
    : gpu::AccelerationStructure(createInfo), device(device), buffer(buffer),
      allocation(allocation), accelerationStructure(accelerationStructure),
      deviceAddress(deviceAddress) {}

AccelerationStructure::~AccelerationStructure() {
  if (accelerationStructure != VK_NULL_HANDLE) {
    vkDestroyAccelerationStructureKHR(device.getVkDevice(),
                                      accelerationStructure, nullptr);
  }
  if (buffer != VK_NULL_HANDLE) {
    vmaDestroyBuffer(device.getVmaAllocator(), buffer, allocation);
  }
}

RayTracingPipeline::RayTracingPipeline(const CreateInfo &createInfo,
                                       Device &device, VkPipeline pipeline,
                                       const LayoutInfo &layoutInfo)
    : gpu::RayTracingPipeline(createInfo), device(device), pipeline(pipeline),
      layoutInfo(layoutInfo) {}

RayTracingPipeline::~RayTracingPipeline() {
  if (pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device.getVkDevice(), pipeline, nullptr);
  }
  if (layoutInfo.pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device.getVkDevice(), layoutInfo.pipelineLayout,
                            nullptr);
  }
  if (layoutInfo.uniformSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.uniformSetLayout, nullptr);
  }
  if (layoutInfo.resourceSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.resourceSetLayout, nullptr);
  }
  if (layoutInfo.emptySetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device.getVkDevice(),
                                 layoutInfo.emptySetLayout, nullptr);
  }
}

UInt32 RayTracingPipeline::getShaderGroupHandleSize() const {
  return device.getRayTracingPipelineProperties().shaderGroupHandleSize;
}

bool RayTracingPipeline::getShaderGroupHandles(UInt32 firstGroup,
                                               UInt32 groupCount, void *dst,
                                               Size dstSize) const {
  if (!pipeline || !dst) {
    return false;
  }
  const Size expectedSize =
      static_cast<Size>(getShaderGroupHandleSize()) * groupCount;
  if (dstSize < expectedSize) {
    return false;
  }
  return vkGetRayTracingShaderGroupHandlesKHR(device.getVkDevice(), pipeline,
                                              firstGroup, groupCount, dstSize,
                                              dst) == VK_SUCCESS;
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
