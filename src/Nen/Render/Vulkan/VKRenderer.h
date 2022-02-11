#pragma once
#include <Nen.hpp>
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

struct InstanceData {
  matrix4 world;
};

class VKRenderer : public IRenderer {
public:
  VKRenderer();
  ~VKRenderer() override {}
  void Initialize(std::shared_ptr<window> window) override;
  void Shutdown() override;
  void Render() override;

  void AddVertexArray(const vertex_array &vArray,
                      std::string_view name) override;
  void UpdateVertexArray(const vertex_array &vArray,
                         std::string_view name) override;

  void AddDrawObject2D(std::shared_ptr<class draw_object> sprite,
                       std::shared_ptr<texture> texture) override;
  void RemoveDrawObject2D(std::shared_ptr<class draw_object> sprite) override;

  void AddDrawObject3D(std::shared_ptr<class draw_object> sprite,
                       std::shared_ptr<texture> texture) override;
  void RemoveDrawObject3D(std::shared_ptr<class draw_object> sprite) override;

  void AddGUI(std::shared_ptr<class ui_screen> ui) override;
  void RemoveGUI(std::shared_ptr<class ui_screen> ui) override;

  void SetRenderer(class renderer *renderer) override;

  void LoadShader(const shader &shaderInfo) override;
  void UnloadShader(const shader &shaderInfo) override;

  nen::renderer *GetRenderer() { return mRenderer; }
  void prepare();
  void cleanup();
  void makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri,
                   VkCommandBufferBeginInfo &ci, VkFence &fence);
  void draw3d(VkCommandBuffer);
  void draw2d(VkCommandBuffer);
  void drawGUI(VkCommandBuffer);
  BufferObject CreateBuffer(
      uint32_t size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  vulkan_base_framework *GetBase() { return m_base.get(); }
  void registerTexture(std::shared_ptr<VulkanDrawObject> texture,
                       std::string_view, texture_type type);
  void unregisterTexture(std::shared_ptr<VulkanDrawObject> texture,
                         texture_type type);
  void registerImageObject(std::shared_ptr<texture>);
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
  VkCommandBuffer CreateCommandBuffer();
  void FinishCommandBuffer(VkCommandBuffer command);
  std::vector<BufferObject> CreateUniformBuffers(uint32_t size,
                                                 uint32_t imageCount);
  const std::vector<std::shared_ptr<VulkanDrawObject>> &GetSprite2Ds() {
    return mDrawObject2D;
  }
  const std::vector<std::shared_ptr<VulkanDrawObject>> &GetSprite3Ds() {
    return mDrawObject3D;
  }
  void AllocateCommandBufferSecondary(uint32_t count,
                                      VkCommandBuffer *pCommands);
  void FreeCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands);
  void TransferStageBufferToImage(const BufferObject &srcBuffer,
                                  const ImageObject &dstImage,
                                  const VkBufferImageCopy *region);
  void write_memory(const VmaAllocation &, const void *data, size_t size);
  std::shared_ptr<class window> GetWindow() { return m_base->m_window; }

  VmaAllocator allocator{};

private:
  renderer *mRenderer;
  std::unique_ptr<class vulkan_base_framework> m_base;
  void prepareUniformBuffers();
  void prepareDescriptorSetLayout();
  void prepareDescriptorPool();
  void prepareDescriptorSetAll();
  void prepareDescriptorSet(std::shared_ptr<VulkanDrawObject>);
  void prepareImGUI();
  void renderImGUI(VkCommandBuffer command);
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
  std::vector<std::pair<shader, Pipeline>> userPipelines;
  std::vector<std::shared_ptr<VulkanDrawObject>> mDrawObject3D;
  std::vector<std::shared_ptr<VulkanDrawObject>> mDrawObject2D;
  std::unordered_map<std::string, ImageObject> mImageObjects;
  std::vector<InstanceData> instance;
  std::vector<BufferObject> m_instanceUniforms;
  std::vector<std::shared_ptr<class ui_screen>> mGUI;
  int instanceCount;
};
} // namespace nen::vk
#endif
