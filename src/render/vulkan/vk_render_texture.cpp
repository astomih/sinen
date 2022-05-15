#include "vk_render_texture.hpp"
#include <array>
#include <vector>

namespace nen {
vk_render_texture::vk_render_texture(VkDevice device, uint32_t width,
                                     uint32_t height)
    : m_device(device) {
  std::array<VkAttachmentDescription, 2> attachments;
  attachments[0] =
      VkAttachmentDescription{0,
                              VK_FORMAT_R8G8B8A8_UNORM,
                              VK_SAMPLE_COUNT_1_BIT,
                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                              VK_ATTACHMENT_STORE_OP_STORE,
                              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                              VK_ATTACHMENT_STORE_OP_DONT_CARE,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  attachments[1] =
      VkAttachmentDescription{0,
                              VK_FORMAT_D32_SFLOAT,
                              VK_SAMPLE_COUNT_1_BIT,
                              VK_ATTACHMENT_LOAD_OP_CLEAR,
                              VK_ATTACHMENT_STORE_OP_STORE,
                              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                              VK_ATTACHMENT_STORE_OP_DONT_CARE,
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference depthRef{
      1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  VkSubpassDescription subpassDesc{0,         VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   0,         nullptr, // InputAttachments
                                   1,         &colorRef,
                                   nullptr,
                                   &depthRef, // DepthStencilAttachments
                                   0,         nullptr};
  VkRenderPassCreateInfo rpCI{
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      nullptr,
      0,
      uint32_t(attachments.size()),
      attachments.data(),
      1,
      &subpassDesc,
      0,
      nullptr,
  };
}
VkFramebuffer vk_render_texture::create_frame_buffer(VkRenderPass renderPass,
                                                     uint32_t width,
                                                     uint32_t height,
                                                     uint32_t viewCount,
                                                     VkImageView *views) {
  VkFramebufferCreateInfo fbCI{
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      nullptr,
      0,
      renderPass,
      viewCount,
      views,
      width,
      height,
      1,
  };
  VkFramebuffer framebuffer;
  vkCreateFramebuffer(m_device, &fbCI, nullptr, &framebuffer);
  return framebuffer;
}
void vk_render_texture::destroy_frame_buffers(uint32_t count,
                                              VkFramebuffer *framebuffers) {
  for (uint32_t i = 0; i < count; ++i) {
    vkDestroyFramebuffer(m_device, framebuffers[i], nullptr);
  }
}

void vk_render_texture::prepare_descriptorsetlayout(
    vk_pipeline_layout &pipeline_layout,
    VkDescriptorSetLayout &descriptor_set_layout) {
  VkDescriptorSetLayoutBinding descSetLayoutBindings[] = {
      {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
      {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
       VK_SHADER_STAGE_FRAGMENT_BIT}};
  VkDescriptorSetLayoutCreateInfo descSetLayoutCI{
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      nullptr,
      0,
      sizeof(descSetLayoutBindings) / sizeof(VkDescriptorSetLayoutBinding),
      descSetLayoutBindings,
  };
  vkCreateDescriptorSetLayout(m_device, &descSetLayoutCI, nullptr,
                              &descriptor_set_layout);
  VkPipelineLayoutCreateInfo pipelineLayoutCI{
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      nullptr,
      0,
      1,
      &descriptor_set_layout,
      0,
      nullptr};
  auto layout = pipeline_layout.GetLayout();
  vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &layout);
}

} // namespace nen