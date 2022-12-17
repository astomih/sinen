#include "../../render/render_system.hpp"
#include "../../script/script_system.hpp"
#include <camera/camera.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <sol/sol.hpp>

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
// general
#include <array>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

// extenal
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// internal
#include "../../texture/texture_system.hpp"
#include "vk_base.hpp"
#include "vk_pipeline_builder.hpp"
#include "vk_renderer.hpp"
#include "vk_shader.hpp"
#include "vk_util.hpp"
#include <io/data_stream.hpp>

namespace sinen {

constexpr int maxpoolSize = 65535;
vk_renderer::vk_renderer()
    : m_descriptor_pool(), m_descriptor_set_layout(), m_sampler(),
      m_base(std::make_unique<vk_base>(this)), m_render_texture(*this),
      m_depth_texture(*this) {}
vk_renderer::~vk_renderer() = default;
void vk_renderer::initialize() {
  m_base->initialize();
  prepare();
}

void vk_renderer::shutdown() {
  cleanup();
  m_base->shutdown();
}
void vk_renderer::render() {
  if (m_base->mSwapchain->is_need_recreate(window::size()))
    m_base->recreate_swapchain();
  uint32_t nextImageIndex = 0;
  m_base->mSwapchain->AcquireNextImage(&nextImageIndex,
                                       m_base->m_presentCompletedSem);
  auto commandFence = m_base->m_fences[nextImageIndex];

  auto color = renderer::clear_color();
  std::array<VkClearValue, 2> clearValue = {{
      {color.r, color.g, color.b, 1.f}, // for Color
      {1.f, 0.f}                        // for Depth
  }};
  VkCommandBufferBeginInfo commandBI{};
  commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  auto &command = m_base->m_commands[nextImageIndex];
  {
    VkRenderPassBeginInfo renderPassShadowBI{};
    renderPassShadowBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassShadowBI.renderPass = m_depth_texture.render_pass;
    renderPassShadowBI.framebuffer = m_depth_texture.fb;
    renderPassShadowBI.renderArea.offset = VkOffset2D{0, 0};
    renderPassShadowBI.renderArea.extent =
        m_base->mSwapchain->GetSurfaceExtent();
    renderPassShadowBI.pClearValues = clearValue.data();
    renderPassShadowBI.clearValueCount = uint32_t(clearValue.size());

    // Begin Command Buffer
    vkBeginCommandBuffer(command, &commandBI);
    m_base->m_imageIndex = nextImageIndex;
    vkCmdBeginRenderPass(command, &renderPassShadowBI,
                         VK_SUBPASS_CONTENTS_INLINE);
    draw_depth(command);
    vkCmdEndRenderPass(command);
    set_image_memory_barrier(command, m_depth_texture.color_target.image,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  {
    VkRenderPassBeginInfo renderPassBI{};
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.renderArea.offset = VkOffset2D{0, 0};
    renderPassBI.renderArea.extent = m_base->mSwapchain->GetSurfaceExtent();
    renderPassBI.pClearValues = clearValue.data();
    renderPassBI.clearValueCount = uint32_t(clearValue.size());
    renderPassBI.renderPass = m_base->m_renderPass;
    renderPassBI.framebuffer = m_base->m_framebuffers[nextImageIndex];
    vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
    m_depth_texture.pipeline.Bind(command);
    VkDeviceSize offset = 0;
    auto &sprite = m_depth_texture.drawer;
    vkCmdBindVertexBuffers(
        command, 0, 1, &m_vertex_arrays["SPRITE"].vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(command, m_vertex_arrays["SPRITE"].indexBuffer.buffer,
                         offset, VK_INDEX_TYPE_UINT32);

    // Set descriptors
    vkCmdBindDescriptorSets(
        command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
        0, 1, &sprite.descripterSet[m_base->m_imageIndex], 0, nullptr);
    auto allocation = sprite.uniformBuffers[m_base->m_imageIndex].allocation;

    sprite.drawable_obj->param.proj = matrix4::identity;
    sprite.drawable_obj->param.view = matrix4::identity;
    sprite.drawable_obj->param.world = matrix4::identity;
    write_memory(allocation, &sprite.drawable_obj->param,
                 sizeof(vk_shader_parameter));

    vkCmdDrawIndexed(command, m_vertex_arrays["SPRITE"].indexCount, 1, 0, 0, 0);
    vkCmdEndRenderPass(command);
    set_image_memory_barrier(command, m_depth_texture.color_target.image,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  {
    VkRenderPassBeginInfo renderPassBI{};
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.renderPass = m_render_texture.render_pass;
    renderPassBI.framebuffer = m_render_texture.fb;
    renderPassBI.renderArea.offset = VkOffset2D{0, 0};
    renderPassBI.renderArea.extent = m_base->mSwapchain->GetSurfaceExtent();
    renderPassBI.pClearValues = clearValue.data();
    renderPassBI.clearValueCount = uint32_t(clearValue.size());
    vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
    make_command(command);
    vkCmdEndRenderPass(command);
    set_image_memory_barrier(command, m_render_texture.color_target.image,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }

  {
    VkRenderPassBeginInfo renderPassBI{};
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.renderArea.offset = VkOffset2D{0, 0};
    renderPassBI.renderArea.extent = m_base->mSwapchain->GetSurfaceExtent();
    renderPassBI.pClearValues = clearValue.data();
    renderPassBI.clearValueCount = uint32_t(clearValue.size());
    renderPassBI.renderPass = m_base->m_renderPass;
    renderPassBI.framebuffer = m_base->m_framebuffers[nextImageIndex];

    // Begin Render Pass
    vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
    render_to_display(command);
    // End Render Pass
    vkCmdEndRenderPass(command);
  }
  vkEndCommandBuffer(command);
  // Do command
  VkSubmitInfo submitInfo{};
  VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command;
  submitInfo.pWaitDstStageMask = &waitStageMask;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &m_base->m_presentCompletedSem;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &m_base->m_renderCompletedSem;
  vkQueueSubmit(m_base->m_deviceQueue, 1, &submitInfo, commandFence);
  m_base->mSwapchain->QueuePresent(m_base->m_deviceQueue, nextImageIndex,
                                   m_base->m_renderCompletedSem);
}
void vk_renderer::add_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
  vk_vertex_array vArrayVK;
  vArrayVK.indexCount = vArray.indexCount;
  vArrayVK.indices = vArray.indices;
  vArrayVK.vertices = vArray.vertices;
  vArrayVK.materialName = vArray.materialName;
  auto vArraySize = vArray.vertices.size() * sizeof(vertex);
  vArrayVK.vertexBuffer =
      create_buffer(vArraySize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  vArrayVK.indexBuffer = create_buffer(vArray.indices.size() * sizeof(uint32_t),
                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  write_memory(vArrayVK.vertexBuffer.allocation, vArrayVK.vertices.data(),
               vArraySize);
  write_memory(vArrayVK.indexBuffer.allocation, vArrayVK.indices.data(),
               sizeof(uint32_t) * vArrayVK.indices.size());
  m_vertex_arrays.insert(
      std::pair<std::string, vk_vertex_array>(name.data(), vArrayVK));
}
void vk_renderer::update_vertex_array(const vertex_array &vArray,
                                      std::string_view name) {
  vk_vertex_array vArrayVK;
  vArrayVK.indexCount = vArray.indexCount;
  vArrayVK.indices = vArray.indices;
  vArrayVK.vertices = vArray.vertices;
  vArrayVK.materialName = vArray.materialName;
  write_memory(m_vertex_arrays[name.data()].vertexBuffer.allocation,
               vArrayVK.vertices.data(),
               vArray.vertices.size() * sizeof(vertex));
  write_memory(m_vertex_arrays[name.data()].indexBuffer.allocation,
               vArrayVK.indices.data(),
               vArray.indices.size() * sizeof(uint32_t));
}
void vk_renderer::add_model(const model &m) {
  vk_vertex_array vArrayVK;
  vArrayVK.indexCount = m.all_indices().size();
  vArrayVK.indices = m.all_indices();
  vArrayVK.vertices = m.all_vertex();
  vArrayVK.materialName = m.v_array.materialName;
  auto vArraySize = m.all_vertex().size() * sizeof(vertex);
  vArrayVK.vertexBuffer =
      create_buffer(vArraySize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  vArrayVK.indexBuffer =
      create_buffer(m.all_indices().size() * sizeof(uint32_t),
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  write_memory(vArrayVK.vertexBuffer.allocation, vArrayVK.vertices.data(),
               vArraySize);
  write_memory(vArrayVK.indexBuffer.allocation, vArrayVK.indices.data(),
               sizeof(uint32_t) * vArrayVK.indices.size());
  m_vertex_arrays.insert(
      std::pair<std::string, vk_vertex_array>(m.name.data(), vArrayVK));
}
void vk_renderer::update_model(const model &m) {
  vk_vertex_array vArrayVK;
  vArrayVK.indexCount = m.all_indices().size();
  vArrayVK.indices = m.all_indices();
  vArrayVK.vertices = m.all_vertex();
  vArrayVK.materialName = m.v_array.materialName;
  write_memory(m_vertex_arrays[m.name.data()].vertexBuffer.allocation,
               vArrayVK.vertices.data(),
               m.all_vertex().size() * sizeof(vertex));
  write_memory(m_vertex_arrays[m.name.data()].indexBuffer.allocation,
               vArrayVK.indices.data(),
               m.all_indices().size() * sizeof(uint32_t));
}
void vk_renderer::draw2d(std::shared_ptr<class drawable> drawObject) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = drawObject;
  add_texture(drawObject->binding_texture);
  registerTexture(t, texture_type::Image2D);
}

void vk_renderer::draw3d(std::shared_ptr<class drawable> sprite) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = sprite;
  add_texture(sprite->binding_texture);
  registerTexture(t, texture_type::Image3D);
}

void vk_renderer::load_shader(const shader &shaderInfo) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(m_base->get_vk_device(),
                      shaderInfo.vertex_shader().c_str(),
                      VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(m_base->get_vk_device(),
                      shaderInfo.fragment_shader().c_str(),
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  vk_pipeline pipeline;
  pipeline.initialize(m_pipeline_layout, m_render_texture.render_pass,
                      shaderStages);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
  pipeline.prepare(m_base->get_vk_device());
  vk_shader::clean(m_base->get_vk_device(), shaderStages);
  m_user_pipelines.emplace_back(
      std::pair<shader, vk_pipeline>{shaderInfo, pipeline});
}
void vk_renderer::unload_shader(const shader &shaderInfo) {
  std::erase_if(m_user_pipelines, [&](auto &x) {
    if (x.first == shaderInfo) {
      x.second.Cleanup(m_base->m_device);
      return true;
    };
    return false;
  });
}

void vk_renderer::add_instancing(const instancing &_instancing) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = _instancing.object;
  add_texture(_instancing.object->binding_texture);
  t->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
  for (auto &v : t->uniformBuffers) {
    VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    v = create_buffer(sizeof(vk_shader_parameter),
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
  }
  layouts.push_back(m_descriptor_set_layout);
  prepare_descriptor_set(t);

  vk_instancing vi{_instancing};
  vi.m_vk_draw_object = t;
  vi.instance_buffer = create_buffer(_instancing.size,
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  write_memory(vi.instance_buffer.allocation, _instancing.data.data(),
               _instancing.size);
  if (_instancing.type == object_type::_2D) {
    m_instancies_2d.push_back(vi);
  } else {
    m_instancies_3d.push_back(vi);
  }
}

void vk_renderer::draw_instancing_3d(VkCommandBuffer command,
                                     bool is_change_pipeline) {
  for (auto &_instancing : m_instancies_3d) {
    if (is_change_pipeline) {
      pipeline_instancing_opaque.Bind(command);
    }
    vkCmdBindDescriptorSets(
        command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
        0, 1,
        &_instancing.m_vk_draw_object->descripterSet[m_base->m_imageIndex], 0,
        nullptr);
    auto allocation =
        _instancing.m_vk_draw_object->uniformBuffers[m_base->m_imageIndex]
            .allocation;
    vk_shader_parameter param;
    param.param = _instancing.m_vk_draw_object->drawable_obj->param;
    param.light_proj = light_projection;
    param.light_view = light_view;
    auto *ptr = _instancing.ins.object->shade.get_parameter().get();
    write_memory(allocation, &param, sizeof(vk_shader_parameter));
    write_memory(allocation, ptr,
                 _instancing.ins.object->shade.get_parameter_size(),
                 sizeof(vk_shader_parameter));
    std::string index = _instancing.ins.object->vertexIndex;
    VkBuffer buffers[] = {m_vertex_arrays[index].vertexBuffer.buffer,
                          _instancing.instance_buffer.buffer};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(command, 0, 2, buffers, offsets);
    vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount,
                     _instancing.ins.data.size(), 0, 0, 0);
  }
}
void vk_renderer::draw_instancing_2d(VkCommandBuffer command) {
  for (auto &_instancing : m_instancies_2d) {
    pipeline_instancing_2d.Bind(command);
    vkCmdBindDescriptorSets(
        command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
        0, 1,
        &_instancing.m_vk_draw_object->descripterSet[m_base->m_imageIndex], 0,
        nullptr);
    auto allocation =
        _instancing.m_vk_draw_object->uniformBuffers[m_base->m_imageIndex]
            .allocation;
    auto *ptr = _instancing.ins.object->shade.get_parameter().get();
    write_memory(allocation, &_instancing.m_vk_draw_object->drawable_obj->param,
                 sizeof(vk_shader_parameter));
    write_memory(allocation, ptr,
                 _instancing.ins.object->shade.get_parameter_size(),
                 sizeof(vk_shader_parameter));
    std::string index = _instancing.ins.object->vertexIndex;
    VkBuffer buffers[] = {m_vertex_arrays[index].vertexBuffer.buffer,
                          _instancing.instance_buffer.buffer};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(command, 0, 2, buffers, offsets);
    vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount,
                     _instancing.ins.data.size(), 0, 0, 0);
  }
}

void vk_renderer::prepare() {
  prepare_descriptor_set_layout();
  prepare_descriptor_pool();

  m_sampler = create_sampler();

  layouts.resize(m_base->mSwapchain->GetImageCount());
  for (auto &i : layouts) {
    i = m_descriptor_set_layout;
  }

  m_pipeline_layout.Initialize(m_base->get_vk_device(),
                               &m_descriptor_set_layout,
                               m_base->mSwapchain->GetSurfaceExtent());
  m_pipeline_layout.Prepare(m_base->get_vk_device());
  m_render_texture.prepare(window::size().x, window::size().y, false);
  m_depth_texture.prepare(window::size().x, window::size().y, true);

  vk_pipeline_builder pipeline_builder(m_base->get_vk_device(),
                                       m_pipeline_layout, m_render_texture,
                                       m_depth_texture, m_base->m_renderPass);
  // render texture pipeline
  pipeline_builder.render_texture_pipeline(m_render_texture.pipeline);
  // depth texture pipeline
  pipeline_builder.depth_texture_pipeline(m_depth_texture.pipeline);
  // Opaque pipeline
  pipeline_builder.opaque(pipeline_opaque);
  // alpha pipeline
  pipeline_builder.alpha(pipeline_alpha);
  // 2D pipeline
  pipeline_builder.alpha_2d(pipeline_2d);
  // SkyBox pipeline
  pipeline_builder.skybox(pipeline_skybox);
  pipeline_builder.instancing_opaque(pipeline_instancing_opaque);
  // alpha pipeline
  pipeline_builder.instancing_alpha(pipeline_instancing_alpha);
  // 2D pipeline
  pipeline_builder.instancing_alpha_2d(pipeline_instancing_2d);
  // depth pipeline
  pipeline_builder.depth(pipeline_depth);
  // depth instanced pipeline
  pipeline_builder.depth_instancing(pipeline_depth_instancing);

  prepare_imgui();
}
void vk_renderer::cleanup() {
  VkDevice device = m_base->get_vk_device();

  for (auto &i : m_image_object) {
    destroy_image(i.second);
  }
  vkutil::DestroyVulkanObject<VkSampler>(device, m_sampler, &vkDestroySampler);
  m_pipeline_layout.Cleanup(device);
  pipeline_opaque.Cleanup(device);
  pipeline_alpha.Cleanup(device);
  pipeline_2d.Cleanup(device);
  for (auto &i : m_user_pipelines) {
    i.second.Cleanup(device);
  }
  for (auto &i : m_vertex_arrays) {
    vkutil::DestroyVulkanObject<VkBuffer>(device, i.second.vertexBuffer.buffer,
                                          &vkDestroyBuffer);
    vkutil::DestroyVulkanObject<VkBuffer>(device, i.second.indexBuffer.buffer,
                                          &vkDestroyBuffer);
  }
  vkutil::DestroyVulkanObject<VkDescriptorPool>(device, m_descriptor_pool,
                                                &vkDestroyDescriptorPool);
  vkutil::DestroyVulkanObject<VkDescriptorSetLayout>(
      device, m_descriptor_set_layout, &vkDestroyDescriptorSetLayout);
}

void vk_renderer::draw_depth(VkCommandBuffer command) {
  auto &lua = (*(sol::state *)script_system::get_state());
  lua["light_eye"] = [&](const vector3 &v) { eye = v; };
  lua["light_at"] = [&](const vector3 &v) { at = v; };
  lua["light_width"] = [&](float v) { width = v; };
  lua["light_height"] = [&](float v) { height = v; };
  light_view = matrix4::lookat(eye, at, vector3(0, 0, 1));
  light_projection = matrix4::ortho(width, height, 0.5, 10);

  pipeline_depth.Bind(command);
  draw3d(command, false);
  pipeline_depth_instancing.Bind(command);
  draw_instancing_3d(command, false);
}

void vk_renderer::make_command(VkCommandBuffer command) {

  /* skybox
   */
  draw_skybox(command);
  pipeline_opaque.Bind(command);
  draw3d(command);
  pipeline_instancing_opaque.Bind(command);
  draw_instancing_3d(command);
  draw2d(command);
  draw_instancing_2d(command);
  pipeline_opaque.Bind(command);

  for (auto &sprite : m_draw_object_3d)
    destroy_texture(sprite);
  m_draw_object_3d.clear();
  for (auto &sprite : m_draw_object_2d)
    destroy_texture(sprite);
  m_draw_object_2d.clear();
  for (auto &_instancing : m_instancies_2d) {
    destroy_texture(_instancing.m_vk_draw_object);
    destroy_buffer(_instancing.instance_buffer);
  }
  m_instancies_2d.clear();
  for (auto &_instancing : m_instancies_3d) {
    destroy_texture(_instancing.m_vk_draw_object);
    destroy_buffer(_instancing.instance_buffer);
  }
  m_instancies_3d.clear();
  render_imgui(command);
}
void vk_renderer::draw_skybox(VkCommandBuffer command) {
  pipeline_skybox.Bind(command);
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = std::make_shared<drawable>();
  t->drawable_obj->binding_texture = render_system::get_skybox_texture();
  t->drawable_obj->vertexIndex = "BOX";
  vk_shader_parameter param;
  matrix4 w = matrix4::identity;
  w[0][0] = 5;
  w[1][1] = 5;
  w[2][2] = 5;
  param.param.proj = scene::main_camera().projection();
  param.param.view = matrix4::lookat(vector3(0, 0, 0),
                                     scene::main_camera().target() -
                                         scene::main_camera().position(),
                                     scene::main_camera().up());
  auto &va = m_vertex_arrays["BOX"];

  if (m_image_object.contains(t->drawable_obj->binding_texture.handle)) {
    destroy_image_object(t->drawable_obj->binding_texture.handle);
  }
  create_image_object(t->drawable_obj->binding_texture.handle);
  t->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
  for (auto &v : t->uniformBuffers) {
    VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    v = create_buffer(sizeof(vk_shader_parameter),
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
  }
  layouts.push_back(m_descriptor_set_layout);
  prepare_descriptor_set(t);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(command, 0, 1, &va.vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(command, va.indexBuffer.buffer, offset,
                       VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline_layout.GetLayout(), 0, 1,
                          &t->descripterSet[m_base->m_imageIndex], 0, nullptr);
  auto allocation = t->uniformBuffers[m_base->m_imageIndex].allocation;
  write_memory(allocation, &param, sizeof(vk_shader_parameter));
  vkCmdDrawIndexed(command, va.indexCount, 1, 0, 0, 0);
  destroy_texture(t);
}

void vk_renderer::draw3d(VkCommandBuffer command, bool is_change_pipeline) {
  VkDeviceSize offset = 0;
  for (auto &sprite : m_draw_object_3d) {
    if (is_change_pipeline) {
      if (sprite->drawable_obj->shade.vertex_shader() == "default" &&
          sprite->drawable_obj->shade.fragment_shader() == "default")
        pipeline_opaque.Bind(command);
      else {
        for (auto &i : m_user_pipelines) {
          if (i.first == sprite->drawable_obj->shade)
            i.second.Bind(command);
        }
      }
    }

    std::string index = sprite->drawable_obj->vertexIndex;
    ::vkCmdBindVertexBuffers(
        command, 0, 1, &m_vertex_arrays[index].vertexBuffer.buffer, &offset);
    ::vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer,
                           offset, VK_INDEX_TYPE_UINT32);
    // Set descriptors
    vkCmdBindDescriptorSets(
        command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
        0, 1, &sprite->descripterSet[m_base->m_imageIndex], 0, nullptr);
    auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
    vk_shader_parameter param;
    param.param = sprite->drawable_obj->param;
    param.light_proj = light_projection;
    param.light_view = light_view;
    auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
    write_memory(allocation, &param, sizeof(vk_shader_parameter));
    write_memory(allocation, ptr,
                 sprite->drawable_obj->shade.get_parameter_size(),
                 sizeof(vk_shader_parameter));
    vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount, 1, 0, 0, 0);
  }
}

void vk_renderer::draw2d(VkCommandBuffer command) {
  pipeline_2d.Bind(command);
  VkDeviceSize offset = 0;
  for (auto &sprite : m_draw_object_2d) {
    if (sprite->drawable_obj->shade.vertex_shader() == "default" &&
        sprite->drawable_obj->shade.fragment_shader() == "default")
      pipeline_2d.Bind(command);
    else {
      for (auto &i : m_user_pipelines) {
        if (i.first == sprite->drawable_obj->shade)
          i.second.Bind(command);
      }
    }
    vkCmdBindVertexBuffers(
        command, 0, 1,
        &m_vertex_arrays[sprite->drawable_obj->vertexIndex].vertexBuffer.buffer,
        &offset);
    vkCmdBindIndexBuffer(
        command,
        m_vertex_arrays[sprite->drawable_obj->vertexIndex].indexBuffer.buffer,
        offset, VK_INDEX_TYPE_UINT32);

    // Set descriptors
    vkCmdBindDescriptorSets(
        command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
        0, 1, &sprite->descripterSet[m_base->m_imageIndex], 0, nullptr);
    auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
    auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
    write_memory(allocation, &sprite->drawable_obj->param,
                 sizeof(vk_shader_parameter));
    write_memory(allocation, ptr,
                 sprite->drawable_obj->shade.get_parameter_size(),
                 sizeof(vk_shader_parameter));

    vkCmdDrawIndexed(
        command, m_vertex_arrays[sprite->drawable_obj->vertexIndex].indexCount,
        1, 0, 0, 0);
  }
}

void vk_renderer::render_to_display(VkCommandBuffer command) {
  m_render_texture.pipeline.Bind(command);
  VkDeviceSize offset = 0;
  auto &sprite = m_render_texture.drawer;
  vkCmdBindVertexBuffers(
      command, 0, 1, &m_vertex_arrays["SPRITE"].vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(command, m_vertex_arrays["SPRITE"].indexBuffer.buffer,
                       offset, VK_INDEX_TYPE_UINT32);

  // Set descriptors
  vkCmdBindDescriptorSets(
      command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout.GetLayout(),
      0, 1, &sprite.descripterSet[m_base->m_imageIndex], 0, nullptr);
  auto allocation = sprite.uniformBuffers[m_base->m_imageIndex].allocation;

  sprite.drawable_obj->param.proj = matrix4::identity;
  sprite.drawable_obj->param.view = matrix4::identity;
  sprite.drawable_obj->param.world = matrix4::identity;
  write_memory(allocation, &sprite.drawable_obj->param,
               sizeof(vk_shader_parameter));

  vkCmdDrawIndexed(command, m_vertex_arrays["SPRITE"].indexCount, 1, 0, 0, 0);
}

void vk_renderer::write_memory(VmaAllocation allocation, const void *data,
                               std::size_t size, std::size_t offset) {
  void *p = nullptr;
  char *pc;
  vmaMapMemory(allocator, allocation, &p);
  pc = reinterpret_cast<char *>(p);
  pc += offset;
  p = reinterpret_cast<void *>(pc);
  memcpy(p, data, size);
  vmaUnmapMemory(allocator, allocation);
}

void vk_renderer::prepare_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  io.IniFilename = NULL;
  io.Fonts->AddFontFromFileTTF(
      data_stream::convert_file_path(asset_type::Font,
                                     "mplus/mplus-1p-medium.ttf")
          .data(),
      18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());

  ImGui_ImplSDL2_InitForVulkan((SDL_Window *)window::get_sdl_window());

  uint32_t imageCount = m_base->mSwapchain->GetImageCount();
  ImGui_ImplVulkan_InitInfo info{};
  info.Instance = m_base->m_instance;
  info.PhysicalDevice = m_base->m_physDev;
  info.Device = m_base->get_vk_device();
  info.QueueFamily = m_base->m_graphicsQueueIndex;
  info.Queue = m_base->get_vk_queue();
  info.DescriptorPool = m_descriptor_pool;
  info.MinImageCount = 2;
  info.ImageCount = imageCount;
  if (!ImGui_ImplVulkan_Init(&info, m_base->m_renderPass)) {
    std::exit(1);
  }

  // Transfar font texture
  VkCommandBufferAllocateInfo commandAI{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
      m_base->m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
  VkCommandBufferBeginInfo beginInfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  VkCommandBuffer command;
  vkAllocateCommandBuffers(m_base->get_vk_device(), &commandAI, &command);
  vkBeginCommandBuffer(command, &beginInfo);
  ImGui_ImplVulkan_CreateFontsTexture(command);
  vkEndCommandBuffer(command);

  VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr};
  submitInfo.pCommandBuffers = &command;
  submitInfo.commandBufferCount = 1;

  vkQueueSubmit(m_base->get_vk_queue(), 1, &submitInfo, VK_NULL_HANDLE);

  // Wait for transfar font texture
  vkDeviceWaitIdle(m_base->get_vk_device());
  vkFreeCommandBuffers(m_base->get_vk_device(), m_base->m_commandPool, 1,
                       &command);
}
void vk_renderer::render_imgui(VkCommandBuffer command) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame((SDL_Window *)window::get_sdl_window());
  ImGui::NewFrame();

  if (renderer::is_show_imgui()) {
    for (auto &i : renderer::get_imgui_function()) {
      i();
    }
  }

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command);
}

void vk_renderer::prepare_descriptor_set_layout() {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  VkDescriptorSetLayoutBinding bindingUBO{}, bindingTex{}, bindingInstance{},
      bindingShadow{};
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

  bindingShadow.binding = 3;
  bindingShadow.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindingShadow.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindingShadow.descriptorCount = 1;
  bindings.push_back(bindingShadow);

  VkDescriptorSetLayoutCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ci.bindingCount = uint32_t(bindings.size());
  ci.pBindings = bindings.data();
  vkCreateDescriptorSetLayout(m_base->get_vk_device(), &ci, nullptr,
                              &m_descriptor_set_layout);
}

void vk_renderer::prepare_descriptor_pool() {

  VkDescriptorPoolSize poolSize[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxpoolSize},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxpoolSize},
  };
  VkDescriptorPoolCreateInfo descPoolCI{
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      nullptr,
      VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      maxpoolSize * sizeof(poolSize) / sizeof(poolSize[0]), // maxSets
      sizeof(poolSize) / sizeof(poolSize[0]),
      poolSize,
  };
  vkCreateDescriptorPool(m_base->get_vk_device(), &descPoolCI, nullptr,
                         &m_descriptor_pool);
}

void vk_renderer::prepare_descriptor_set(std::shared_ptr<vk_drawable> sprite) {
  VkDescriptorSetAllocateInfo ai{};
  ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  ai.descriptorPool = m_descriptor_pool;
  ai.descriptorSetCount = uint32_t(m_base->mSwapchain->GetImageCount());
  ai.pSetLayouts = layouts.data();
  sprite->descripterSet.resize(m_base->mSwapchain->GetImageCount());
  vkAllocateDescriptorSets(m_base->get_vk_device(), &ai,
                           sprite->descripterSet.data());
  // Write to descriptor set.
  for (int i = 0; i < m_base->mSwapchain->GetImageCount(); i++) {
    VkDescriptorBufferInfo descUBO{};
    descUBO.buffer = sprite->uniformBuffers[i].buffer;
    descUBO.offset = 0;
    descUBO.range = VK_WHOLE_SIZE;

    std::array<VkDescriptorImageInfo, 2> descImage;

    descImage[0].imageView =
        m_image_object[sprite->drawable_obj->binding_texture.handle].view;
    descImage[0].sampler = m_sampler;
    descImage[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descImage[1].imageView = m_depth_texture.depth_target.view;
    descImage[1].sampler = m_depth_texture.sampler;
    descImage[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet ubo{};
    ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ubo.dstBinding = 0;
    ubo.descriptorCount = 1;
    ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo.pBufferInfo = &descUBO;
    ubo.dstSet = sprite->descripterSet[i];

    VkWriteDescriptorSet tex{};
    tex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tex.dstBinding = 1;
    tex.descriptorCount = 1;
    tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    tex.pImageInfo = &descImage[0];
    tex.dstSet = sprite->descripterSet[i];
    VkWriteDescriptorSet shadowTex{};
    shadowTex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadowTex.dstBinding = 3;
    shadowTex.descriptorCount = 1;
    shadowTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowTex.pImageInfo = &descImage[1];
    shadowTex.dstSet = sprite->descripterSet[i];

    std::vector<VkWriteDescriptorSet> writeSets = {ubo, tex, shadowTex};
    vkUpdateDescriptorSets(m_base->get_vk_device(), uint32_t(writeSets.size()),
                           writeSets.data(), 0, nullptr);
  }
}
vk_buffer_object vk_renderer::create_buffer(uint32_t size,
                                            VkBufferUsageFlags usage,
                                            VkMemoryPropertyFlags flags) {
  vk_buffer_object obj;
  VkBufferCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  ci.usage = usage;
  ci.size = size;
  VmaAllocationCreateInfo buffer_alloc_info = {};
  buffer_alloc_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
  vmaCreateBuffer(allocator, &ci, &buffer_alloc_info, &obj.buffer,
                  &obj.allocation, nullptr);
  return obj;
}

VkSampler vk_renderer::create_sampler() {
  VkSampler sampler;
  VkSamplerCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  ci.minFilter = VK_FILTER_LINEAR;
  ci.magFilter = VK_FILTER_LINEAR;
  ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  ci.maxAnisotropy = 1.0f;
  ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
  vkCreateSampler(m_base->get_vk_device(), &ci, nullptr, &sampler);
  return sampler;
}

void vk_renderer::set_image_memory_barrier(VkCommandBuffer command,
                                           VkImage image,
                                           VkImageLayout oldLayout,
                                           VkImageLayout newLayout) {
  VkImageMemoryBarrier imb{};
  imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imb.oldLayout = oldLayout;
  imb.newLayout = newLayout;
  imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imb.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  imb.image = image;

  VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  switch (oldLayout) {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    imb.srcAccessMask = 0;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  default:
    break;
  }

  switch (newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    break;
  }

  srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  vkCmdPipelineBarrier(command, srcStage, dstStage, 0,
                       0, // memoryBarrierCount
                       nullptr,
                       0, // bufferMemoryBarrierCount
                       nullptr,
                       1, // imageMemoryBarrierCount
                       &imb);
}

uint32_t
vk_renderer::get_memory_type_index(uint32_t requestBits,
                                   VkMemoryPropertyFlags requestProps) const {
  uint32_t result = ~0u;
  for (uint32_t i = 0; i < m_physical_mem_props.memoryTypeCount; ++i) {
    if (requestBits & 1) {
      const auto &types = m_physical_mem_props.memoryTypes[i];
      if ((types.propertyFlags & requestProps) == requestProps) {
        result = i;
        break;
      }
    }
    requestBits >>= 1;
  }
  return result;
}

void vk_renderer::destroy_buffer(vk_buffer_object &bufferObj) {
  vmaDestroyBuffer(allocator, bufferObj.buffer, bufferObj.allocation);
}

void vk_renderer::destroy_image(vk_image_object &imageObj) {
  vkDestroyImageView(m_base->get_vk_device(), imageObj.view, nullptr);
  vmaDestroyImage(allocator, imageObj.image, imageObj.allocation);
}

void vk_renderer::create_image_object(const handle_t &handle) {
  ::SDL_Surface &surf = texture_system::get(handle);
  ::SDL_LockSurface(&surf);
  auto *formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
  formatbuf->BytesPerPixel = 4;
  auto *imagedata = ::SDL_ConvertSurface(&surf, formatbuf, 0);
  ::SDL_UnlockSurface(&surf);
  vk_buffer_object stagingBuffer;
  vk_image_object texture;
  int width = imagedata->w, height = imagedata->h;
  auto *pImage = imagedata->pixels;
  {
    // Create VkImage texture
    VkImageCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.extent = {uint32_t(width), uint32_t(height), 1};
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.arrayLayers = 1;
    ci.mipLevels = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR;
    ci.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vmaCreateImage(allocator, &ci, &alloc_info, &texture.image,
                   &texture.allocation, nullptr);
  }

  {
    uint32_t imageSize = imagedata->h * imagedata->w * 4;
    stagingBuffer = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    write_memory(stagingBuffer.allocation, pImage, imageSize);
  }

  VkBufferImageCopy copyRegion{};
  copyRegion.imageExtent = {uint32_t(width), uint32_t(height), 1};
  copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
  VkCommandBuffer command;
  {
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandBufferCount = 1;
    ai.commandPool = m_base->m_commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_base->get_vk_device(), &ai, &command);
  }

  VkCommandBufferBeginInfo commandBI{};
  commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(command, &commandBI);
  set_image_memory_barrier(command, texture.image,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vkCmdCopyBufferToImage(command, stagingBuffer.buffer, texture.image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

  set_image_memory_barrier(command, texture.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  vkEndCommandBuffer(command);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command;
  vkQueueSubmit(m_base->get_vk_queue(), 1, &submitInfo,
                m_base->m_fences[m_base->m_imageIndex]);

  // Create view for texture reference
  VkImageViewCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ci.image = texture.image;
  ci.format = VK_FORMAT_R8G8B8A8_UNORM;
  ci.components = {
      .r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
      .g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
      .b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
      .a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A,
  };

  ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  vkCreateImageView(m_base->get_vk_device(), &ci, nullptr, &texture.view);
  vkDeviceWaitIdle(m_base->get_vk_device());
  vkFreeCommandBuffers(m_base->get_vk_device(), m_base->m_commandPool, 1,
                       &command);
  destroy_buffer(stagingBuffer);
  SDL_FreeFormat(formatbuf);
  SDL_FreeSurface(imagedata);
  m_image_object.emplace(handle, texture);
}
void vk_renderer::add_texture(texture handle) {
  if (m_image_object.contains(handle.handle)) {
    if (*handle.is_need_update) {
      destroy_image_object(handle.handle);
    } else
      return;
  }
  create_image_object(handle.handle);
}
void vk_renderer::destroy_image_object(const handle_t &handle) {
  for (auto it = m_image_object.begin(); it != m_image_object.end(); ++it) {
    if (it->first == handle) {
      destroy_image(it->second);
      m_image_object.erase(it);
      return;
    }
  }
}
VkRenderPass vk_renderer::get_render_pass(const std::string &name) {
  if (m_base)
    return m_base->m_renderPass;
  return 0;
}
VkDevice vk_renderer::get_device() {
  if (m_base)
    return m_base->get_vk_device();
  return m_base->get_vk_device();
}
void vk_renderer::registerTexture(std::shared_ptr<vk_drawable> texture,
                                  texture_type type) {
  if (texture_type::Image3D == type) {
    m_draw_object_3d.push_back(texture);
    texture->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
    for (auto &v : texture->uniformBuffers) {
      VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      auto shader_param_size =
          texture->drawable_obj->shade.get_parameter_size();
      v = create_buffer(sizeof(vk_shader_parameter) + shader_param_size,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
    }
    layouts.push_back(m_descriptor_set_layout);
    prepare_descriptor_set(texture);
  } else {
    auto iter = m_draw_object_2d.begin();
    for (; iter != m_draw_object_2d.end(); ++iter) {
      if (texture->drawable_obj->drawOrder < (*iter)->drawable_obj->drawOrder) {
        break;
      }
    }

    // Inserts element before position of iterator
    m_draw_object_2d.insert(iter, texture);
    texture->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
    for (auto &v : texture->uniformBuffers) {
      VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      v = create_buffer(sizeof(vk_shader_parameter) +
                            texture->drawable_obj->shade.get_parameter_size(),
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
    }
    layouts.push_back(m_descriptor_set_layout);
    prepare_descriptor_set(texture);
  }
}
void vk_renderer::destroy_texture(std::shared_ptr<vk_drawable> texture) {
  auto device = m_base->get_vk_device();
  vkFreeDescriptorSets(device, m_descriptor_pool,
                       static_cast<uint32_t>(texture->descripterSet.size()),
                       texture->descripterSet.data());
  for (auto &i : texture->uniformBuffers) {
    destroy_buffer(i);
  }
  texture->uniformBuffers.clear();
  layouts.pop_back();
}
} // namespace sinen
#endif
