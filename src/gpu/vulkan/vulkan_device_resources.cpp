#ifndef EMSCRIPTEN
#include "vulkan_command.hpp"
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"
#include "vulkan_resources.hpp"

#include <SDL3/SDL.h>

#include <cstring>

namespace sinen::gpu::vulkan {
void Device::createDefaultResources() {
  {
    gpu::Sampler::CreateInfo sCI{};
    sCI.allocator = getCreateInfo().allocator;
    sCI.minFilter = gpu::Filter::Linear;
    sCI.magFilter = gpu::Filter::Linear;
    sCI.mipmapMode = gpu::MipmapMode::Linear;
    sCI.addressModeU = gpu::AddressMode::Repeat;
    sCI.addressModeV = gpu::AddressMode::Repeat;
    sCI.addressModeW = gpu::AddressMode::Repeat;
    sCI.enableAnisotropy = false;
    sCI.enableCompare = false;
    defaultSamplerObject = createSampler(sCI);
    defaultSampler = downCast<Sampler>(defaultSamplerObject)->getNative();
  }

  {
    gpu::Texture::CreateInfo tCI{};
    tCI.allocator = getCreateInfo().allocator;
    tCI.type = gpu::TextureType::Texture2D;
    tCI.format = gpu::TextureFormat::R8G8B8A8_UNORM;
    tCI.usage = gpu::TextureUsage::Sampler;
    tCI.width = 1;
    tCI.height = 1;
    tCI.layerCountOrDepth = 1;
    tCI.numLevels = 1;
    tCI.sampleCount = gpu::SampleCount::x1;
    defaultTexture = downCast<Texture>(createTexture(tCI));
    defaultTextureView = defaultTexture->getView();

    uint32_t pixel = 0xFFFFFFFFu;
    gpu::TransferBuffer::CreateInfo tbCI{};
    tbCI.allocator = getCreateInfo().allocator;
    tbCI.usage = gpu::TransferBufferUsage::Upload;
    tbCI.size = sizeof(pixel);
    auto staging = createTransferBuffer(tbCI);
    void *mapped = staging->map(false);
    std::memcpy(mapped, &pixel, sizeof(pixel));
    staging->unmap();

    gpu::CommandBuffer::CreateInfo cbCI{};
    cbCI.allocator = getCreateInfo().allocator;
    auto cb = acquireCommandBuffer(cbCI);
    auto copyPass = cb->beginCopyPass();
    gpu::TextureRegion region{};
    region.texture = defaultTexture;
    region.width = 1;
    region.height = 1;
    region.depth = 1;
    gpu::TextureTransferInfo src{};
    src.transferBuffer = staging;
    src.offset = 0;
    copyPass->uploadTexture(src, region, false);
    cb->endCopyPass(copyPass);
    submitCommandBuffer(cb);
  }
}

Ptr<gpu::Buffer> Device::createBuffer(const Buffer::CreateInfo &createInfo) {
  VkBufferUsageFlags usage =
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (createInfo.usage == gpu::BufferUsage::Vertex)
    usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (createInfo.usage == gpu::BufferUsage::Index)
    usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (createInfo.usage == gpu::BufferUsage::Indirect)
    usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

  VkBufferCreateInfo bufferCI{};
  bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCI.size = createInfo.size;
  bufferCI.usage = usage;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocCI{};
  allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  if (vmaCreateBuffer(vmaAllocator, &bufferCI, &allocCI, &buffer, &allocation,
                      nullptr) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vmaCreateBuffer failed");
    return nullptr;
  }

  return makePtr<Buffer>(createInfo.allocator, createInfo, *this, buffer,
                         allocation);
}

Ptr<gpu::TransferBuffer>
Device::createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) {
  VkBufferUsageFlags usage =
      (createInfo.usage == gpu::TransferBufferUsage::Upload)
          ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT
          : VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  VkBufferCreateInfo bufferCI{};
  bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCI.size = createInfo.size;
  bufferCI.usage = usage;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocCI{};
  allocCI.usage = VMA_MEMORY_USAGE_AUTO;
  allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                  VMA_ALLOCATION_CREATE_MAPPED_BIT;

  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  VmaAllocationInfo info{};
  if (vmaCreateBuffer(vmaAllocator, &bufferCI, &allocCI, &buffer, &allocation,
                      &info) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vmaCreateBuffer (transfer) failed");
    return nullptr;
  }
  return makePtr<TransferBuffer>(createInfo.allocator, createInfo, *this,
                                 buffer, allocation, info.pMappedData);
}

Ptr<gpu::Texture> Device::createTexture(const Texture::CreateInfo &createInfo) {
  VkFormat format = convert::textureFormatFrom(createInfo.format);
  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if (createInfo.usage == gpu::TextureUsage::Sampler)
    usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  if (createInfo.usage == gpu::TextureUsage::ColorTarget)
    usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (createInfo.usage == gpu::TextureUsage::DepthStencilTarget)
    usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkImageType imageType = VK_IMAGE_TYPE_2D;
  VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
  uint32_t layers = 1;
  uint32_t depth = 1;
  switch (createInfo.type) {
  case gpu::TextureType::Texture2D:
    break;
  case gpu::TextureType::Texture2DArray:
    viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    layers = createInfo.layerCountOrDepth;
    break;
  case gpu::TextureType::Texture3D:
    imageType = VK_IMAGE_TYPE_3D;
    viewType = VK_IMAGE_VIEW_TYPE_3D;
    depth = createInfo.layerCountOrDepth;
    break;
  case gpu::TextureType::Cube:
    viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    layers = 6;
    break;
  case gpu::TextureType::CubeArray:
    viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    layers = createInfo.layerCountOrDepth;
    break;
  }

  VkImageCreateInfo imageCI{};
  imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCI.imageType = imageType;
  imageCI.format = format;
  imageCI.extent = {createInfo.width, createInfo.height, depth};
  imageCI.mipLevels = createInfo.numLevels;
  imageCI.arrayLayers = layers;
  imageCI.samples = convert::sampleCountFrom(createInfo.sampleCount);
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = usage;
  imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  if (createInfo.type == gpu::TextureType::Cube ||
      createInfo.type == gpu::TextureType::CubeArray) {
    imageCI.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }

  VmaAllocationCreateInfo allocCI{};
  allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  VkImage image = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  if (vmaCreateImage(vmaAllocator, &imageCI, &allocCI, &image, &allocation,
                     nullptr) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: vmaCreateImage failed");
    return nullptr;
  }

  VkImageViewCreateInfo viewCI{};
  viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCI.image = image;
  viewCI.viewType = viewType;
  viewCI.format = format;
  viewCI.subresourceRange.aspectMask = convert::aspectMaskFrom(format);
  viewCI.subresourceRange.baseMipLevel = 0;
  viewCI.subresourceRange.levelCount = createInfo.numLevels;
  viewCI.subresourceRange.baseArrayLayer = 0;
  viewCI.subresourceRange.layerCount = layers;

  VkImageView view = VK_NULL_HANDLE;
  if (vkCreateImageView(device, &viewCI, nullptr, &view) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateImageView failed");
    vmaDestroyImage(vmaAllocator, image, allocation);
    return nullptr;
  }

  Texture::NativeCreateInfo native{};
  native.image = image;
  native.view = view;
  native.format = format;
  native.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  native.ownsImage = true;
  native.ownsView = true;
  native.allocation = allocation;
  native.width = createInfo.width;
  native.height = createInfo.height;
  native.depth = depth;
  native.layers = layers;
  native.mipLevels = createInfo.numLevels;

  return makePtr<Texture>(createInfo.allocator, createInfo, *this, native);
}

Ptr<gpu::Sampler> Device::createSampler(const Sampler::CreateInfo &createInfo) {
  VkSamplerCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  ci.magFilter = convert::filterFrom(createInfo.magFilter);
  ci.minFilter = convert::filterFrom(createInfo.minFilter);
  ci.mipmapMode = convert::mipmapModeFrom(createInfo.mipmapMode);
  ci.addressModeU = convert::addressModeFrom(createInfo.addressModeU);
  ci.addressModeV = convert::addressModeFrom(createInfo.addressModeV);
  ci.addressModeW = convert::addressModeFrom(createInfo.addressModeW);
  ci.mipLodBias = createInfo.mipLodBias;
  ci.anisotropyEnable = createInfo.enableAnisotropy ? VK_TRUE : VK_FALSE;
  ci.maxAnisotropy = createInfo.maxAnisotropy;
  ci.compareEnable = createInfo.enableCompare ? VK_TRUE : VK_FALSE;
  ci.compareOp = convert::compareOpFrom(createInfo.compareOp);
  ci.minLod = createInfo.minLod;
  ci.maxLod = createInfo.maxLod;
  ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  ci.unnormalizedCoordinates = VK_FALSE;

  VkSampler sampler = VK_NULL_HANDLE;
  if (vkCreateSampler(device, &ci, nullptr, &sampler) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateSampler failed");
    return nullptr;
  }
  return makePtr<Sampler>(createInfo.allocator, createInfo, *this, sampler);
}

Ptr<gpu::Shader> Device::createShader(const Shader::CreateInfo &createInfo) {
  if (createInfo.format != gpu::ShaderFormat::SPIRV) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Vulkan: only SPIRV supported");
    return nullptr;
  }
  VkShaderModuleCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  ci.codeSize = createInfo.size;
  ci.pCode = reinterpret_cast<const uint32_t *>(createInfo.data);
  VkShaderModule module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &ci, nullptr, &module) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateShaderModule failed");
    return nullptr;
  }
  return makePtr<Shader>(createInfo.allocator, createInfo, *this, module);
}

gpu::TextureFormat Device::getSwapchainFormat() const {
  if (swapchainVkFormat == VK_FORMAT_B8G8R8A8_UNORM) {
    return gpu::TextureFormat::B8G8R8A8_UNORM;
  }
  return gpu::TextureFormat::R8G8B8A8_UNORM;
}

void Device::waitForGpuIdle() { vkDeviceWaitIdle(device); }

String Device::getDriver() const {
  return String("vulkan", getCreateInfo().allocator);
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
