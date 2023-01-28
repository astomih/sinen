#ifndef SINEN_VK_RENDERER_HPP
#define SINEN_VK_RENDERER_HPP
#include <functional>
#include <instancing/instancing.hpp>
#include <memory>
#include <model/model.hpp>
#include <texture/texture_type.hpp>
#include <vertex/vertex_array.hpp>

#if !defined(EMSCRIPTEN) && !defined(ANDROID)

#include "vk_base.hpp"
#include "vk_buffer.hpp"
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

class vk_instancing {
public:
  vk_instancing(const instancing &_instancing) : ins(_instancing) {}
  instancing ins;
  std::shared_ptr<class vk_drawable> m_vk_draw_object;
  vk_buffer instance_buffer;
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

  void draw2d(std::shared_ptr<class drawable> sprite);
  void drawui(std::shared_ptr<class drawable> sprite);
  void draw3d(std::shared_ptr<class drawable> sprite);

  void load_shader(const shader &shaderInfo);
  void unload_shader(const shader &shaderInfo);

  void add_instancing(const instancing &_instancing);

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
  std::vector<VkDescriptorSetLayout> &get_layouts() { return layouts; };

  VkDescriptorPool get_descriptor_pool() const { return m_descriptor_pool; }
  void destroy_buffer(vk_buffer &bufferObj);
  void destroy_image(vk_image &imageObj);
  void write_memory(VmaAllocation, const void *data, std::size_t size,
                    std::size_t offset = 0);

  VmaAllocator allocator{};
  vk_render_texture m_render_texture;

private:
  std::unique_ptr<class vk_base> m_base;
  void prepare_descriptor_set_layout();
  void prepare_descriptor_pool();
  void prepare_descriptor_set(std::shared_ptr<vk_drawable>);
  void prepare_imgui();
  void render_imgui(VkCommandBuffer command);
  void draw_skybox(VkCommandBuffer command);
  void draw_instancing_3d(VkCommandBuffer command,
                          bool is_change_pipeline = true);
  void draw_instancing_2d(VkCommandBuffer command);
  void create_image_object(const handle_t &handle);
  VkSampler create_sampler();
  void set_image_memory_barrier(VkCommandBuffer command, VkImage image,
                                VkImageLayout oldLayout,
                                VkImageLayout newLayout);
  std::unordered_map<std::string, vk_vertex_array> m_vertex_arrays;
  std::vector<VkDescriptorSetLayout> layouts;
  VkDescriptorSetLayout m_descriptor_set_layout;
  VkDescriptorPool m_descriptor_pool;
  VkSampler m_sampler;
  VkPhysicalDeviceMemoryProperties m_physical_mem_props;
  vk_pipeline_layout m_pipeline_layout_normal;
  vk_pipeline_layout m_pipeline_layout_instance;
  std::unordered_map<std::string, vk_pipeline> m_pipelines;
  std::vector<std::pair<shader, vk_pipeline>> m_user_pipelines;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_3d;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_2d;
  std::vector<std::shared_ptr<vk_drawable>> m_draw_object_ui;
  std::unordered_map<handle_t, vk_image> m_image_object;
  std::vector<vk_instancing> m_instancies_3d;
  std::vector<vk_instancing> m_instancies_2d;
  std::shared_ptr<vk_drawable> m_skybox;
};
} // namespace sinen
#endif

#endif
