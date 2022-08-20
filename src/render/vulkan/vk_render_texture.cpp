#include "vk_render_texture.hpp"
#include "vk_renderer.hpp"
#include <array>
#include <vector>

namespace sinen {
vk_render_texture::vk_render_texture(vk_renderer &r) : m_vkrenderer(r) {}

void vk_render_texture::prepare(int width, int height, bool depth_only) {
  sampler = create_sampler();
  drawer.drawObject = std::make_shared<drawable>();
  is_depth_only = depth_only;
  if (!depth_only) {
    color_target =
        create_image_object(width, height, VK_FORMAT_R8G8B8A8_UNORM, false);
  }
  depth_target = create_image_object(width, height, VK_FORMAT_D32_SFLOAT, true);
  VkRenderPassCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

  if (depth_only) {
    VkAttachmentDescription attachment;
    attachment =
        VkAttachmentDescription{0,
                                VK_FORMAT_D32_SFLOAT,
                                VK_SAMPLE_COUNT_1_BIT,
                                VK_ATTACHMENT_LOAD_OP_CLEAR,
                                VK_ATTACHMENT_STORE_OP_STORE,
                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthReference{};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 0;
    subpassDesc.pColorAttachments = nullptr;
    subpassDesc.pDepthStencilAttachment = &depthReference;

    ci.attachmentCount = 1;
    ci.pAttachments = &attachment;
    ci.subpassCount = 1;
    ci.pSubpasses = &subpassDesc;
    vkCreateRenderPass(m_vkrenderer.get_base().m_device, &ci, nullptr,
                       &render_pass);
    std::vector<VkImageView> views;
    views.push_back(depth_target.view);
    VkFramebufferCreateInfo fbci{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        render_pass,
        (uint32_t)views.size(),
        views.data(),
        (uint32_t)width,
        (uint32_t)height,
        1,
    };
    // VkFramebuffer m_renderTextureFB;
    vkCreateFramebuffer(m_vkrenderer.get_base().m_device, &fbci, nullptr, &fb);
    prepare_descriptor_set();
  } else {
    std::array<VkAttachmentDescription, 2> attachments;
    auto &colorTarget = attachments[0];
    auto &depthTarget = attachments[1];

    colorTarget = VkAttachmentDescription{

        0,                        // Flags
        VK_FORMAT_R8G8B8A8_UNORM, // Format
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    depthTarget =
        VkAttachmentDescription{0,
                                VK_FORMAT_D32_SFLOAT,
                                VK_SAMPLE_COUNT_1_BIT,
                                VK_ATTACHMENT_LOAD_OP_CLEAR,
                                VK_ATTACHMENT_STORE_OP_STORE,
                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL};

    VkAttachmentReference colorReference{}, depthReference{};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorReference;
    subpassDesc.pDepthStencilAttachment = &depthReference;

    ci.attachmentCount = uint32_t(attachments.size());
    ci.pAttachments = attachments.data();
    ci.subpassCount = 1;
    ci.pSubpasses = &subpassDesc;

    vkCreateRenderPass(m_vkrenderer.get_base().m_device, &ci, nullptr,
                       &render_pass);
    std::vector<VkImageView> views;
    views.push_back(color_target.view);
    views.push_back(depth_target.view);
    VkFramebufferCreateInfo fbci{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        render_pass,
        (uint32_t)views.size(),
        views.data(),
        (uint32_t)width,
        (uint32_t)height,
        1,
    };
    // VkFramebuffer m_renderTextureFB;
    vkCreateFramebuffer(m_vkrenderer.get_base().m_device, &fbci, nullptr, &fb);
    prepare_descriptor_set();
  }
}
vk_image_object vk_render_texture::create_image_object(int width, int height,
                                                       VkFormat format,
                                                       bool isdepth) {
  vk_image_object tex{};
  {
    // Create VkImage texture
    VkImageCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.extent = {uint32_t(width), uint32_t(height), 1};
    ci.format = format;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.arrayLayers = 1;
    ci.mipLevels = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (isdepth)
      ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_SAMPLED_BIT;
    else
      ci.usage =
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    alloc_info.flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.requiredFlags =
        VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    vmaCreateImage(m_vkrenderer.allocator, &ci, &alloc_info, &tex.image,
                   &tex.allocation, nullptr);
  }

  {
    // Create view for texture reference
    VkImageViewCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.image = tex.image;
    ci.format = format;
    ci.components = {
        .r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
        .g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
        .b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
        .a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A,
    };

    ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkCreateImageView(m_vkrenderer.get_base().get_vk_device(), &ci, nullptr,
                      &tex.view);
  }
  return tex;
}
void vk_render_texture::prepare_descriptor_set() {
  prepare_descriptor_set_layout();
  drawer.uniformBuffers.resize(
      m_vkrenderer.get_base().mSwapchain->GetImageCount());
  for (auto &v : drawer.uniformBuffers) {
    VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    v = m_vkrenderer.create_buffer(
        sizeof(shader_parameter), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
  }
  std::vector<VkDescriptorSetLayout> layouts;
  for (size_t i = 0; i < drawer.uniformBuffers.size(); i++) {
    layouts.push_back(descriptor_set_layout);
  }

  VkDescriptorSetAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  ai.descriptorPool = m_vkrenderer.get_descriptor_pool();
  ai.descriptorSetCount =
      uint32_t(m_vkrenderer.get_base().mSwapchain->GetImageCount());
  ai.pSetLayouts = layouts.data();
  drawer.descripterSet.resize(
      m_vkrenderer.get_base().mSwapchain->GetImageCount());
  vkAllocateDescriptorSets(m_vkrenderer.get_base().get_vk_device(), &ai,
                           drawer.descripterSet.data());
  // Write to descriptor set.
  for (int i = 0; i < m_vkrenderer.get_base().mSwapchain->GetImageCount();
       i++) {
    VkDescriptorBufferInfo descUBO{};
    descUBO.buffer = drawer.uniformBuffers[i].buffer;
    descUBO.offset = 0;
    descUBO.range = VK_WHOLE_SIZE;

    VkDescriptorImageInfo descImage;
    descImage.imageView = color_target.view;
    descImage.sampler = sampler;
    descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet ubo{};
    ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ubo.dstBinding = 0;
    ubo.descriptorCount = 1;
    ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo.pBufferInfo = &descUBO;
    ubo.dstSet = drawer.descripterSet[i];

    VkWriteDescriptorSet tex{};
    tex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tex.dstBinding = 1;
    tex.descriptorCount = 1;
    tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    if (is_depth_only) {
      tex.pImageInfo = nullptr;
    } else
      tex.pImageInfo = &descImage;
    tex.dstSet = drawer.descripterSet[i];

    std::vector<VkWriteDescriptorSet> writeSets;
    writeSets.push_back(ubo);
    if (!is_depth_only)
      writeSets.push_back(tex);
    vkUpdateDescriptorSets(m_vkrenderer.get_base().get_vk_device(),
                           uint32_t(writeSets.size()), writeSets.data(), 0,
                           nullptr);
  }
}
VkSampler vk_render_texture::create_sampler() {
  VkSampler sampler;
  VkSamplerCreateInfo ci{
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      nullptr,
      0,
      VK_FILTER_LINEAR,
      VK_FILTER_LINEAR,
      VK_SAMPLER_MIPMAP_MODE_LINEAR,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      0.0f,
      VK_FALSE,
      1.0f,
      VK_FALSE,
      VK_COMPARE_OP_NEVER,
      0.0f,
      0.0f,
      VK_BORDER_COLOR_INT_OPAQUE_WHITE,
      VK_FALSE,
  };
  vkCreateSampler(m_vkrenderer.get_base().get_vk_device(), &ci, nullptr,
                  &sampler);
  return sampler;
}
void vk_render_texture::prepare_descriptor_set_layout() {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  VkDescriptorSetLayoutBinding bindingUBO{}, bindingTex{}, bindingInstance{};
  bindingUBO.binding = 0;
  bindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindingUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  bindingUBO.descriptorCount = 1;
  bindings.push_back(bindingUBO);

  bindingTex.binding = 1;
  bindingTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindingTex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindingTex.descriptorCount = 1;
  bindings.push_back(bindingTex);

  bindingInstance.binding = 2;
  bindingInstance.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindingInstance.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  bindingInstance.descriptorCount = 1;
  bindings.push_back(bindingInstance);

  VkDescriptorSetLayoutCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ci.bindingCount = uint32_t(bindings.size());
  ci.pBindings = bindings.data();
  vkCreateDescriptorSetLayout(m_vkrenderer.get_base().get_vk_device(), &ci,
                              nullptr, &descriptor_set_layout);
}

} // namespace sinen