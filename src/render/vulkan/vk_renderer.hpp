#pragma once
#include "instancing/instancing.hpp"
#include <functional>
#include <memory>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <render/renderer.hpp>

#include "vk_base.hpp"
#include "vk_pipeline.hpp"
#include "vk_pipeline_layout.hpp"
#include <array>
#include <draw_object/draw_object.hpp>
#include <string_view>
#include <unordered_map>
#include <vk_mem_alloc.h>

namespace nen::vk {
struct vk_buffer_object {
  VkBuffer buffer;
  VmaAllocation allocation;
};
struct vk_image_object {
  VkImage image;
  VmaAllocation allocation;
  VkImageView view;
};

class vk_draw_object {
public:
  std::vector<VkDescriptorSet> descripterSet;
  std::vector<vk_buffer_object> uniformBuffers;
  bool isInstance = false;
  std::shared_ptr<nen::draw_object> drawObject;
};

struct vk_vertex_array : public vertex_array {
  vk_buffer_object vertexBuffer;
  vk_buffer_object indexBuffer;
};

class vk_instancing {
public:
  vk_instancing(const instancing &_instancing) : ins(_instancing) {}
  instancing ins;
  std::shared_ptr<vk_draw_object> vk_draw_object;
  vk_buffer_object instance_buffer;
};

class vk_renderer : public renderer::Interface {
public:
  vk_renderer(manager &_manager);
  ~vk_renderer() override {}
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

  void add_instancing(const instancing &_instancing) override;

  void prepare();
  void cleanup();
  void makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri,
                   VkCommandBufferBeginInfo &ci, VkFence &fence);
  void draw3d(VkCommandBuffer);
  void draw2d(VkCommandBuffer);
  vk_buffer_object CreateBuffer(
      uint32_t size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  vk_base *GetBase() { return m_base.get(); }
  void registerTexture(std::shared_ptr<vk_draw_object> texture,
                       texture_type type);
  void unregisterTexture(std::shared_ptr<vk_draw_object> texture);
  void registerImageObject(const handle_t &handle);
  void unregister_image_object(const handle_t &handle);
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
  void DestroyBuffer(vk_buffer_object &bufferObj);
  void DestroyImage(vk_image_object &imageObj);
  void DestroyFramebuffers(uint32_t count, VkFramebuffer *framebuffers);
  void write_memory(VmaAllocation, const void *data, size_t size);

  VmaAllocator allocator{};

private:
  std::unique_ptr<class vk_base> m_base;
  void prepareDescriptorSetLayout();
  void prepareDescriptorPool();
  void prepareDescriptorSet(std::shared_ptr<vk_draw_object>);
  void prepareImGUI();
  void renderImGUI(VkCommandBuffer command);
  void draw_skybox(VkCommandBuffer command);
  void draw_instancing_3d(VkCommandBuffer command);
  void draw_instancing_2d(VkCommandBuffer command);
  void update_image_object(const handle_t &handle);
  VkSampler createSampler();
  vk_image_object create_texture(SDL_Surface *imagedata, VkFormat format);
  vk_image_object createTextureFromSurface(const ::SDL_Surface &surface);
  vk_image_object createTextureFromMemory(const std::vector<char> &imageData);
  void setImageMemoryBarrier(VkCommandBuffer command, VkImage image,
                             VkImageLayout oldLayout, VkImageLayout newLayout);
  std::unordered_map<std::string, vk_vertex_array> m_VertexArrays;
  vk_image_object m_shadowColor;
  vk_image_object m_shadowDepth;
  std::vector<float> m_faceWeights;
  VkFramebuffer m_shadowFramebuffer;
  std::vector<VkDescriptorSetLayout> layouts;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkDescriptorPool m_descriptorPool;
  VkSampler m_sampler;
  VkPhysicalDeviceMemoryProperties m_physicalMemProps;
  vk_pipeline_layout mPipelineLayout;
  vk_pipeline pipelineSkyBox;
  vk_pipeline pipelineOpaque;
  vk_pipeline pipelineAlpha;
  vk_pipeline pipeline2D;
  vk_pipeline pipelineInstancingOpaque;
  vk_pipeline pipelineInstancingAlpha;
  vk_pipeline pipelineInstancing2D;
  std::vector<std::pair<shader, vk_pipeline>> userPipelines;
  std::vector<std::shared_ptr<vk_draw_object>> mDrawObject3D;
  std::vector<std::shared_ptr<vk_draw_object>> mDrawObject2D;
  std::unordered_map<handle_t, vk_image_object> mImageObjects;
  std::vector<vk_instancing> m_instancies_3d;
  std::vector<vk_instancing> m_instancies_2d;
  vk_buffer_object m_instance_buffer;
  manager &m_manager;
};
} // namespace nen::vk
#endif
