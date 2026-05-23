#pragma once
#ifndef EMSCRIPTEN

#include <core/data/ptr.hpp>
#include <gpu/gpu_buffer.hpp>
#include <gpu/gpu_compute_pipeline.hpp>
#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_ray_tracing.hpp>
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
         VmaAllocation allocation, VkDeviceAddress deviceAddress = 0);
  ~Buffer() override;

  VkBuffer getNative() const { return buffer; }
  VkDeviceAddress getDeviceAddress() const { return deviceAddress; }

private:
  Device &device;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  VkDeviceAddress deviceAddress = 0;
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
  ShaderStage getStage() const { return stage; }
  uint32_t getNumUniformBuffers() const { return numUniformBuffers; }
  uint32_t getNumSamplers() const { return numSamplers; }
  uint32_t getNumStorageBuffers() const { return numStorageBuffers; }
  uint32_t getNumStorageTextures() const { return numStorageTextures; }

private:
  Device &device;
  VkShaderModule module = VK_NULL_HANDLE;
  const char *entrypoint = "main";
  ShaderStage stage = ShaderStage::Vertex;
  uint32_t numUniformBuffers = 0;
  uint32_t numSamplers = 0;
  uint32_t numStorageBuffers = 0;
  uint32_t numStorageTextures = 0;
};

class GraphicsPipeline : public gpu::GraphicsPipeline {
public:
  struct LayoutInfo {
    VkDescriptorSetLayout vertexSamplerSetLayout = VK_NULL_HANDLE;   // set = 0
    VkDescriptorSetLayout vertexUniformSetLayout = VK_NULL_HANDLE;   // set = 1
    VkDescriptorSetLayout fragmentSamplerSetLayout = VK_NULL_HANDLE; // set = 2
    VkDescriptorSetLayout fragmentUniformSetLayout = VK_NULL_HANDLE; // set = 3
    VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE; // set = 4-5
    VkDescriptorSetLayout accelerationStructureSetLayout =
        VK_NULL_HANDLE; // set = 6
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t vertexUniformBindingCount = 0;
    uint32_t fragmentUniformBindingCount = 0;
    uint32_t fragmentSamplerBindingCount = 0;
  };

  GraphicsPipeline(const CreateInfo &createInfo, Device &device,
                   VkPipeline pipeline, VkRenderPass renderPass,
                   const LayoutInfo &layoutInfo);
  ~GraphicsPipeline() override;

  VkPipeline getNative() const { return pipeline; }
  VkRenderPass getRenderPass() const { return renderPass; }
  const LayoutInfo &getLayoutInfo() const { return layoutInfo; }

private:
  Device &device;
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  LayoutInfo layoutInfo{};
};

class ComputePipeline : public gpu::ComputePipeline {
public:
  struct LayoutInfo {
    VkDescriptorSetLayout storageBufferSetLayout = VK_NULL_HANDLE; // set = 0
    VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE;       // set = 1
    VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE;         // set = 2-5
    VkDescriptorSetLayout accelerationStructureSetLayout =
        VK_NULL_HANDLE; // set = 6
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    uint32_t storageBufferBindingCount = 0;
    uint32_t uniformBindingCount = 0;
  };

  ComputePipeline(const CreateInfo &createInfo, Device &device,
                  VkPipeline pipeline, const LayoutInfo &layoutInfo);
  ~ComputePipeline() override;

  VkPipeline getNative() const { return pipeline; }
  const LayoutInfo &getLayoutInfo() const { return layoutInfo; }

private:
  Device &device;
  VkPipeline pipeline = VK_NULL_HANDLE;
  LayoutInfo layoutInfo{};
};

class AccelerationStructure : public gpu::AccelerationStructure {
public:
  AccelerationStructure(const CreateInfo &createInfo, Device &device,
                        VkBuffer buffer, VmaAllocation allocation,
                        VkAccelerationStructureKHR accelerationStructure,
                        VkDeviceAddress deviceAddress);
  ~AccelerationStructure() override;

  VkAccelerationStructureKHR getNative() const { return accelerationStructure; }
  UInt64 getDeviceAddress() const override { return deviceAddress; }

private:
  Device &device;
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
  VkDeviceAddress deviceAddress = 0;
};

class RayTracingPipeline : public gpu::RayTracingPipeline {
public:
  struct LayoutInfo {
    VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout resourceSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  };

  RayTracingPipeline(const CreateInfo &createInfo, Device &device,
                     VkPipeline pipeline, const LayoutInfo &layoutInfo);
  ~RayTracingPipeline() override;

  VkPipeline getNative() const { return pipeline; }
  const LayoutInfo &getLayoutInfo() const { return layoutInfo; }
  UInt32 getShaderGroupHandleSize() const override;
  bool getShaderGroupHandles(UInt32 firstGroup, UInt32 groupCount, void *dst,
                             Size dstSize) const override;

private:
  Device &device;
  VkPipeline pipeline = VK_NULL_HANDLE;
  LayoutInfo layoutInfo{};
};

} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
