#ifndef SINEN_VK_RENDER_TEXTURE_HPP
#define SINEN_VK_RENDER_TEXTURE_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include "vk_drawable.hpp"
#include "vk_image.hpp"
#include "vk_pipeline.hpp"
#include <vulkan/vulkan.h>

namespace sinen {
class vk_render_texture {
public:
  vk_render_texture(class vk_renderer &);
  void set_MSAA(bool enable);
  void prepare_descriptor_set_layout();
  void prepare_descriptor_set();
  void prepare(int width, int height);
  void clear();
  void window_resize(int width, int height);
  VkSampler create_sampler();
  vk_image create_image_object(int width, int height, VkFormat format,
                               bool isdepth);
  void destroy_image_object(vk_image &image_object);
  void create_frame_buffer(int width, int height);
  void destroy_frame_buffer();
  void prepare_descriptor_set_for_imgui();
  void destroy_descriptor_set_for_imgui();

  VkRenderPass render_pass;
  VkFramebuffer fb;
  VkSampler sampler;
  vk_image color_target;
  vk_image depth_target;
  vk_drawable drawer;
  VkDescriptorSetLayout descriptor_set_layout;
  VkDescriptorSet imgui_descriptor_set;
  vk_pipeline pipeline;

private:
  class vk_renderer &m_vkrenderer;
  bool is_MSAA = false;
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(ANDROID)
#endif // !SINEN_VK_RENDER_TEXTURE_HPP
