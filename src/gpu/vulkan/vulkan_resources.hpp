#pragma once
#ifndef EMSCRIPTEN

#include <core/data/ptr.hpp>
#include <gpu/gpu_buffer.hpp>
#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_sampler.hpp>
#include <gpu/gpu_shader.hpp>
#include <gpu/gpu_texture.hpp>
#include <gpu/gpu_transfer_buffer.hpp>

#include "vma.hpp"

namespace sinen::gpu::vulkan {
class Device;

class Buffer : public gpu::Buffer {
public:
  Buffer(const CreateInfo &createInfo, Device &device, VkBuffer buffer,
         VmaAllocation allocation);
  ~Buffer() override;

  VkBuffer getNative() const { return buffer; }

private:
  Device &device;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
};

class TransferBuffer : public gpu::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, Device &device, VkBuffer buffer,
                 VmaAllocation allocation, void *mapped);
  ~TransferBuffer() override;

  VkBuffer getNative() const { return buffer; }
  VmaAllocation getAllocation() const { return allocation; }

  void *map(bool cycle) override;
  void unmap() override;

private:
  Device &device;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  void *mapped = nullptr;
  bool mappedNow = false;
};

class Texture : public gpu::Texture {
public:
  struct NativeCreateInfo {
    VkImage image;
    VkImageView view;
    VkFormat format;
    VkImageLayout initialLayout;
    bool ownsImage;
    bool ownsView;
    VmaAllocation allocation;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t layers;
    uint32_t mipLevels;
  };

  Texture(const CreateInfo &createInfo, Device &device,
          const NativeCreateInfo &native);
  ~Texture() override;

  VkImage getImage() const { return image; }
  VkImageView getView() const { return view; }
  VkFormat getFormat() const { return format; }
  VkImageLayout getLayout() const { return layout; }
  void setLayout(VkImageLayout newLayout) { layout = newLayout; }

  VkImageAspectFlags getAspectMask() const;
  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }
  uint32_t getDepth() const { return depth; }
  uint32_t getLayers() const { return layers; }
  uint32_t getMipLevels() const { return mipLevels; }

private:
  Device &device;
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  bool ownsImage = false;
  bool ownsView = false;
  VmaAllocation allocation = VK_NULL_HANDLE;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t layers = 1;
  uint32_t mipLevels = 1;
};

class Sampler : public gpu::Sampler {
public:
  Sampler(const CreateInfo &createInfo, Device &device, VkSampler sampler);
  ~Sampler() override;

  VkSampler getNative() const { return sampler; }

private:
  Device &device;
  VkSampler sampler = VK_NULL_HANDLE;
};

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, Device &device, VkShaderModule module);
  ~Shader() override;

  VkShaderModule getNative() const { return module; }
  const char *getEntrypoint() const { return entrypoint; }
  gpu::ShaderStage getStage() const { return stage; }
  uint32_t getNumUniformBuffers() const { return numUniformBuffers; }
  uint32_t getNumSamplers() const { return numSamplers; }

private:
  Device &device;
  VkShaderModule module = VK_NULL_HANDLE;
  const char *entrypoint = "main";
  gpu::ShaderStage stage = gpu::ShaderStage::Vertex;
  uint32_t numUniformBuffers = 0;
  uint32_t numSamplers = 0;
};

class GraphicsPipeline : public gpu::GraphicsPipeline {
public:
  struct LayoutInfo {
    VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE;   // set = 0
    VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE; // set = 1
    VkDescriptorSetLayout samplerSetLayout = VK_NULL_HANDLE; // set = 2
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t uniformBindingCount = 0;
    uint32_t samplerBindingCount = 0;
  };

  GraphicsPipeline(const CreateInfo &createInfo, Device &device,
                   VkPipeline pipeline, const LayoutInfo &layoutInfo);
  ~GraphicsPipeline() override;

  VkPipeline getNative() const { return pipeline; }
  const LayoutInfo &getLayoutInfo() const { return layoutInfo; }

private:
  Device &device;
  VkPipeline pipeline = VK_NULL_HANDLE;
  LayoutInfo layoutInfo{};
};

} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
