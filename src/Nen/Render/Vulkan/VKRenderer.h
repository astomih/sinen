#pragma once
#include "instancing/instancing.hpp"
#include <Nen.hpp>
#include <functional>
#include <memory>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "VKBase.h"
#include <DrawObject/DrawObject.hpp>
#include <array>
#include <string_view>
#include <unordered_map>
#include <vk_mem_alloc.h>

namespace nen::vk {
struct BufferObject {
  VkBuffer buffer;
  VmaAllocation allocation;
};
struct ImageObject {
  VkImage image;
  VmaAllocation allocation;
  VkImageView view;
};

class VulkanDrawObject {
public:
  std::vector<VkDescriptorSet> descripterSet;
  std::vector<BufferObject> uniformBuffers;
  bool isInstance = false;
  std::shared_ptr<nen::draw_object> drawObject;
};

struct VertexArrayForVK : public vertex_array {
  BufferObject vertexBuffer;
  BufferObject indexBuffer;
};

class vulkan_instancing {
public:
  vulkan_instancing(instancing &_instancing) : ins(_instancing) {}
  instancing &ins;
  std::shared_ptr<VulkanDrawObject> vk_draw_object;
  BufferObject instance_buffer;
};

class VKRenderer : public IRenderer {
public:
  VKRenderer(manager &_manager);
  ~VKRenderer() override {}
  void Initialize() override;
  void Shutdown() override;
  void Render() override;

  void AddVertexArray(const vertex_array &vArray,
                      std::string_view name) override;
  void UpdateVertexArray(const vertex_array &vArray,
                         std::string_view name) override;

  void draw2d(std::shared_ptr<class draw_object> sprite) override;
  void draw3d(std::shared_ptr<class draw_object> sprite) override;

  void LoadShader(const shader &shaderInfo) override;
  void UnloadShader(const shader &shaderInfo) override;

  void add_instancing(instancing &_instancing) override;
  void remove_instancing(instancing &_instancing) override;

  void prepare();
  void cleanup();
  void makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri,
                   VkCommandBufferBeginInfo &ci, VkFence &fence);
  void draw3d(VkCommandBuffer);
  void draw2d(VkCommandBuffer);
  BufferObject CreateBuffer(
      uint32_t size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  vulkan_base_framework *GetBase() { return m_base.get(); }
  void registerTexture(std::shared_ptr<VulkanDrawObject> texture,
                       std::string_view, texture_type type);
  void unregisterTexture(std::shared_ptr<VulkanDrawObject> texture,
                         texture_type type);
  void registerImageObject(const handle_t &handle);
  VkPipelineLayout GetPipelineLayout(const std::string &name) {
    return mPipelineLayout.GetLayout();
  }
  VkDescriptorSetLayout GetDescriptorSetLayout(const std::string &name) {
    return m_descriptorSetLayout;
  }
  VkRenderPass GetRenderPass(const std::string &name);
  VkDescriptorPool GetDescriptorPool() const { return m_descriptorPool; }
  VkDevice GetDevice();
  uint32_t GetMemoryTypeIndex(uint32_t requestBits,
                              VkMemoryPropertyFlags requestProps) const;
  void RegisterRenderPass(const std::string &name, VkRenderPass renderPass);
  VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, uint32_t width,
                                  uint32_t height, uint32_t viewCount,
                                  VkImageView *views);
  void DestroyBuffer(BufferObject bufferObj);
  void DestroyImage(ImageObject imageObj);
  void DestroyFramebuffers(uint32_t count, VkFramebuffer *framebuffers);
  void write_memory(const VmaAllocation &, const void *data, size_t size);

  VmaAllocator allocator{};

private:
  std::unique_ptr<class vulkan_base_framework> m_base;
  void prepareUniformBuffers();
  void prepareDescriptorSetLayout();
  void prepareDescriptorPool();
  void prepareDescriptorSetAll();
  void prepareDescriptorSet(std::shared_ptr<VulkanDrawObject>);
  void prepareImGUI();
  void renderImGUI(VkCommandBuffer command);
  void draw_instancing_3d(VkCommandBuffer command);
  void draw_instancing_2d(VkCommandBuffer command);
  VkSampler createSampler();
  ImageObject create_texture(SDL_Surface *imagedata, VkFormat format);
  ImageObject createTextureFromSurface(const ::SDL_Surface &surface);
  ImageObject createTextureFromMemory(const std::vector<char> &imageData);
  void setImageMemoryBarrier(VkCommandBuffer command, VkImage image,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  std::unordered_map<std::string, VertexArrayForVK> m_VertexArrays;
  ImageObject m_shadowColor;
  ImageObject m_shadowDepth;
  std::vector<float> m_faceWeights;
  VkFramebuffer m_shadowFramebuffer;
  std::vector<VkDescriptorSetLayout> layouts;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorPool m_descriptorPool;
  VkSampler m_sampler;
  VkPhysicalDeviceMemoryProperties m_physicalMemProps;
  PipelineLayout mPipelineLayout;
  Pipeline pipelineOpaque;
  Pipeline pipelineAlpha;
  Pipeline pipeline2D;
  Pipeline pipelineInstancingOpaque;
  Pipeline pipelineInstancingAlpha;
  Pipeline pipelineInstancing2D;
  std::vector<std::pair<shader, Pipeline>> userPipelines;
  std::vector<std::shared_ptr<VulkanDrawObject>> mDrawObject3D;
  std::vector<std::shared_ptr<VulkanDrawObject>> mDrawObject2D;
  std::unordered_map<std::string, ImageObject> mImageObjects;
  std::vector<vulkan_instancing> m_instancies_3d;
  std::vector<vulkan_instancing> m_instancies_2d;
  BufferObject m_instance_buffer;
  manager &m_manager;
};
} // namespace nen::vk
#endif
