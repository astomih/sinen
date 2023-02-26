#ifndef SINEN_VK_RENDERER_HPP
#define SINEN_VK_RENDERER_HPP
#include <functional>
#include <memory>
#include <model/model.hpp>
#include <vertex/vertex_array.hpp>

#if !defined(EMSCRIPTEN) && !defined(ANDROID)

#include "vk_base.hpp"
#include "vk_buffer.hpp"
#include "vk_depth_texture.hpp"
#include "vk_drawable.hpp"
#include "vk_image.hpp"
#include "vk_pipeline.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_render_texture.hpp"
#include "vma.hpp"
#include <array>
#include <drawable/drawable.hpp>
#include <string_view>
#include <unordered_map>

namespace sinen {

struct vk_vertex_array : public vertex_array {
  vk_buffer vertexBuffer;
  vk_buffer indexBuffer;
};

class vk_shader_parameter {
public:
  drawable::parameter param;
};

class vk_renderer {
public:
  vk_renderer();
  ~vk_renderer();
  void initialize();
  void shutdown();
  void render();
  void render_to_display(VkCommandBuffer command);

  void add_vertex_array(const vertex_array &vArray, std::string_view name);
  void update_vertex_array(const vertex_array &vArray, std::string_view name);
  void add_model(const model &m);
  void update_model(const model &m);

  void draw2d(std::shared_ptr<struct drawable> sprite);
  void drawui(std::shared_ptr<struct drawable> sprite);
  void draw3d(std::shared_ptr<struct drawable> sprite);

  void load_shader(const shader &shaderInfo);
  void unload_shader(const shader &shaderInfo);

  void prepare();
  void cleanup();
  void make_command(VkCommandBuffer command);
  void draw3d(VkCommandBuffer, bool is_change_pipeline = true);
  void draw2d(VkCommandBuffer);
  void drawui(VkCommandBuffer);
  vk_buffer create_buffer(
      uint32_t size, VkBufferUsageFlags usage,
      VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      VmaMemoryUsage vma_usage = VMA_MEMORY_USAGE_GPU_TO_CPU);
  vk_base &get_base() { return *m_base; }
  void register_vk_drawable(std::shared_ptr<class vk_drawable> texture,
                            std::string_view type);
  void destroy_vk_drawable(std::shared_ptr<class vk_drawable> texture);
  void add_texture(texture tex);
  void destroy_image_object(const handle_t &handle);
  VkPipelineLayout get_pipeline_layout() {
    return m_pipeline_layout_instance.GetLayout();
  }
  VkDescriptorSetLayout get_descriptor_set_layout() {
    return m_descriptor_set_layout;
  }
  VkDescriptorPool get_descriptor_pool() const { return m_descriptor_pool; }
  void destroy_buffer(vk_buffer &bufferObj);
  void destroy_image(vk_image &imageObj);
  void write_memory(VmaAllocation, const void *data, std::size_t size,
                    std::size_t offset = 0);
  void *get_texture_id() { return m_present_texture.imgui_descriptor_set; }

  VmaAllocator allocator{};
  vk_render_texture m_render_texture;
  vk_render_texture m_present_texture;
  vk_depth_texture m_depth_texture;

private:
  std::unique_ptr<class vk_base> m_base;
  void prepare_descriptor_set_layout();
  void prepare_descriptor_pool();
  void prepare_descriptor_set(std::shared_ptr<vk_drawable>);
  void prepare_imgui();
  void render_imgui(VkCommandBuffer command);
  void draw_skybox(VkCommandBuffer command);
  void create_image_object(const handle_t &handle);
  VkSampler create_sampler();
  void set_image_memory_barrier(VkCommandBuffer command, VkImage image,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout);
  std::unordered_map<std::string, vk_vertex_array> m_vertex_arrays;
  VkDescriptorSetLayout m_descriptor_set_layout;
  VkDescriptorSetLayout m_descriptor_set_layout_for_depth;
  VkDescriptorPool m_descriptor_pool;
  VkSampler m_sampler;
  vk_pipeline_layout m_pipeline_layout_normal;
  vk_pipeline_layout m_pipeline_layout_instance;
  vk_pipeline_layout m_pipeline_layout_depth;
  vk_pipeline_layout m_pipeline_layout_depth_instance;
  std::unordered_map<std::string, vk_pipeline> m_pipelines;
  std::vector<std::pair<shader, vk_pipeline>> m_user_pipelines;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_3d;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_2d;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_ui;
  std::unordered_map<handle_t, vk_image> m_image_object;
  std::shared_ptr<vk_drawable> m_skybox;
};
} // namespace sinen
#endif

#endif
