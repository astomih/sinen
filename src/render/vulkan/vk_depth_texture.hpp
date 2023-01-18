#ifndef SINEN_VK_DEPTH_TEXTURE_HPP
#define SINEN_VK_DEPTH_TEXTURE_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include "vk_object.hpp"
#include "vk_pipeline.hpp"
#include "vk_pipeline_layout.hpp"
#include <vulkan/vulkan.h>

namespace sinen {
class vk_depth_texture {
public:
  vk_depth_texture(class vk_renderer &);

  void prepare_descriptor_set_layout();
  void prepare_descriptor_set();
  void prepare(int width, int height);
  VkSampler create_sampler();
  vk_image_object create_image_object(int width, int height, VkFormat format,
                                      bool isdepth);

  VkRenderPass render_pass;
  VkFramebuffer fb;
  VkSampler sampler;
  vk_image_object color_target;
  vk_image_object depth_target;
  vk_drawable drawer;
  VkDescriptorSetLayout descriptor_set_layout;
  vk_pipeline pipeline;

private:
  class vk_renderer &m_vkrenderer;
};
} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(ANDROID)
#endif // !SINEN_VK_DEPTH_TEXTURE_HPP