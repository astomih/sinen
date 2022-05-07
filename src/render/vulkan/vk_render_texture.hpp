#ifndef SINEN_VK_RENDER_TEXTURE_HPP
#define SINEN_VK_RENDER_TEXTURE_HPP
#include "vk_pipeline_layout.hpp"
#include <vulkan/vulkan.h>

namespace nen {
class vk_render_texture {
public:
  vk_render_texture(VkDevice device, uint32_t width, uint32_t height);
  VkFramebuffer create_frame_buffer(VkRenderPass renderPass, uint32_t width,
                                    uint32_t height, uint32_t viewCount,
                                    VkImageView *views);
  void
  prepare_descriptorsetlayout(vk_pipeline_layout &pipeline_layout,
                              VkDescriptorSetLayout &descriptor_set_layout);
  void destroy_frame_buffers(uint32_t count, VkFramebuffer *framebuffers);
  VkDevice m_device;
};
} // namespace nen
#endif