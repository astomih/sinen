#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include <logger/logger.hpp>

#include "../../render/render_system.hpp"
#include <camera/camera.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <sol/sol.hpp>
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

// internal
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
      m_present_texture(*this), m_depth_texture(*this) {}
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
  /* Skybox */
  m_skybox->drawable_obj->binding_texture = render_system::get_skybox_texture();
  if (m_image_object.contains(m_skybox->drawable_obj->binding_texture.handle)) {
    destroy_image_object(m_skybox->drawable_obj->binding_texture.handle);
  }
  create_image_object(m_skybox->drawable_obj->binding_texture.handle);
  for (auto &v : m_skybox->uniformBuffers) {
    VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    v = create_buffer(sizeof(vk_shader_parameter),
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
  }
  prepare_descriptor_set(m_skybox);
  uint32_t nextImageIndex = 0;
  m_base->mSwapchain->acquire_next_image(&nextImageIndex,
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
  // Begin Command Buffer
  vkBeginCommandBuffer(command, &commandBI);
  {
    VkRenderPassBeginInfo renderPassBI{};
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.renderPass = m_depth_texture.render_pass;
    renderPassBI.framebuffer = m_depth_texture.fb;
    renderPassBI.renderArea.offset = VkOffset2D{0, 0};
    renderPassBI.renderArea.extent = m_base->mSwapchain->GetSurfaceExtent();
    renderPassBI.pClearValues = clearValue.data();
    renderPassBI.clearValueCount = uint32_t(clearValue.size());
    vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
    VkRect2D scissor = {{0, 0}, m_base->mSwapchain->GetSurfaceExtent()};
    vkCmdSetScissor(command, 0, 1, &scissor);
    VkViewport viewport = {
        0,
        float(m_base->mSwapchain->GetSurfaceExtent().height),
        float(m_base->mSwapchain->GetSurfaceExtent().width),
        -float(m_base->mSwapchain->GetSurfaceExtent().height),
        0,
        1};
    vkCmdSetViewport(command, 0, 1, &viewport);
    VkDeviceSize offset = 0;
    for (auto &sprite : m_draw_object_3d) {
      if (sprite->drawable_obj->size() > 0) {
        m_pipelines["depth_instancing"].Bind(command);
        vkCmdBindDescriptorSets(
            command, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline_layout_depth_instance.GetLayout(), 0, 1,
            &sprite->descriptor_sets_for_depth[m_base->m_imageIndex], 0,
            nullptr);
        auto allocation =
            sprite->uniformBuffers[m_base->m_imageIndex].allocation;
        vk_shader_parameter param;
        param.param = sprite->drawable_obj->param;
        param.param.light_proj = renderer::light_projection;
        param.param.light_view = renderer::light_view;
        auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
        write_memory(allocation, &param, sizeof(vk_shader_parameter));
        write_memory(allocation, ptr,
                     sprite->drawable_obj->shade.get_parameter_size(),
                     sizeof(vk_shader_parameter));
        std::string index = sprite->drawable_obj->vertexIndex;
        VkBuffer buffers[] = {m_vertex_arrays[index].vertexBuffer.buffer,
                              sprite->instance_buffer.buffer};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(command, 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer,
                             0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount,
                         sprite->drawable_obj->data.size(), 0, 0, 0);
      } else {

        m_pipelines["depth"].Bind(command);

        std::string index = sprite->drawable_obj->vertexIndex;
        ::vkCmdBindVertexBuffers(command, 0, 1,
                                 &m_vertex_arrays[index].vertexBuffer.buffer,
                                 &offset);
        ::vkCmdBindIndexBuffer(command,
                               m_vertex_arrays[index].indexBuffer.buffer,
                               offset, VK_INDEX_TYPE_UINT32);
        // Set descriptors
        vkCmdBindDescriptorSets(
            command, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline_layout_depth.GetLayout(), 0, 1,
            &sprite->descriptor_sets_for_depth[m_base->m_imageIndex], 0,
            nullptr);
        auto allocation =
            sprite->uniformBuffers[m_base->m_imageIndex].allocation;
        vk_shader_parameter param;
        param.param = sprite->drawable_obj->param;
        param.param.light_proj = renderer::light_projection;
        param.param.light_view = renderer::light_view;
        auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
        write_memory(allocation, &param, sizeof(vk_shader_parameter));
        write_memory(allocation, ptr,
                     sprite->drawable_obj->shade.get_parameter_size(),
                     sizeof(vk_shader_parameter));
        vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount, 1, 0, 0,
                         0);
      }
    }
    vkCmdEndRenderPass(command);
    set_image_memory_barrier(command, m_depth_texture.color_target.image,
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
    VkRect2D scissor = {{0, 0}, m_base->mSwapchain->GetSurfaceExtent()};
    vkCmdSetScissor(command, 0, 1, &scissor);
    VkViewport viewport = {
        0,
        float(m_base->mSwapchain->GetSurfaceExtent().height),
        float(m_base->mSwapchain->GetSurfaceExtent().width),
        -float(m_base->mSwapchain->GetSurfaceExtent().height),
        0,
        1};
    vkCmdSetViewport(command, 0, 1, &viewport);
    make_command(command);
    vkCmdEndRenderPass(command);
    set_image_memory_barrier(command, m_render_texture.color_target.image,
                             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  }
  set_image_memory_barrier(command, m_present_texture.color_target.image,
                           VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  set_image_memory_barrier(command, m_render_texture.color_target.image,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  // Resolve
  VkImageResolve resolveMSAA = {{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                {0, 0, 0}, // srcOffset
                                {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                                {0, 0, 0}, // dstOffset
                                {m_base->mSwapchain->GetSurfaceExtent().width,
                                 m_base->mSwapchain->GetSurfaceExtent().height,
                                 1}};
  vkCmdResolveImage(command, m_render_texture.color_target.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_present_texture.color_target.image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &resolveMSAA);
  VkImageMemoryBarrier barrier = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // sType
      nullptr,                                  // pNext
      VK_ACCESS_TRANSFER_WRITE_BIT,             // srcAccessMask
      VK_ACCESS_MEMORY_READ_BIT,                // dstAccessMask
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // oldLayout
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // newLayout
      VK_QUEUE_FAMILY_IGNORED,                  // srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED,                  // dstQueueFamilyIndex
      m_present_texture.color_target.image,     // image
      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}   // subresourceRange
  };
  vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                       &barrier);

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
    VkViewport viewport = {
        0,
        float(m_base->mSwapchain->GetSurfaceExtent().height),
        float(m_base->mSwapchain->GetSurfaceExtent().width),
        -float(m_base->mSwapchain->GetSurfaceExtent().height),
        0,
        1};
    vkCmdSetViewport(command, 0, 1, &viewport);
    if (!renderer::offscreen_rendering) {
      m_present_texture.pipeline.Bind(command);
      VkDeviceSize offset = 0;
      auto &sprite = m_present_texture.drawer;
      vkCmdBindVertexBuffers(command, 0, 1,
                             &m_vertex_arrays["SPRITE"].vertexBuffer.buffer,
                             &offset);
      vkCmdBindIndexBuffer(command,
                           m_vertex_arrays["SPRITE"].indexBuffer.buffer, offset,
                           VK_INDEX_TYPE_UINT32);

      // Set descriptors
      vkCmdBindDescriptorSets(
          command, VK_PIPELINE_BIND_POINT_GRAPHICS,
          m_present_texture.m_pipeline_layout.GetLayout(), 0, 1,
          &sprite.descriptor_sets[m_base->m_imageIndex], 0, nullptr);
      auto allocation = sprite.uniformBuffers[m_base->m_imageIndex].allocation;

      sprite.drawable_obj->param.proj = matrix4::identity;
      sprite.drawable_obj->param.view = matrix4::identity;
      sprite.drawable_obj->param.world = matrix4::identity;
      sprite.drawable_obj->param.user = renderer::render_texture_user_data;
      write_memory(allocation, &sprite.drawable_obj->param,
                   sizeof(vk_shader_parameter));

      vkCmdDrawIndexed(command, m_vertex_arrays["SPRITE"].indexCount, 1, 0, 0,
                       0);
    }
    drawui(command);
    render_imgui(command);
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
  vkResetFences(m_base->m_device, 1, &commandFence);
  vkQueueSubmit(m_base->m_deviceQueue, 1, &submitInfo, commandFence);
  m_base->mSwapchain->queue_present(m_base->m_deviceQueue, nextImageIndex,
                                    m_base->m_renderCompletedSem);
  vkQueueWaitIdle(m_base->m_deviceQueue);
  destroy_vk_drawable(m_skybox);
  for (auto &sprite : m_draw_object_3d)
    destroy_vk_drawable(sprite);
  m_draw_object_3d.clear();
  for (auto &sprite : m_draw_object_2d)
    destroy_vk_drawable(sprite);
  m_draw_object_2d.clear();
  for (auto &sprite : m_draw_object_ui)
    destroy_vk_drawable(sprite);
  m_draw_object_ui.clear();
  if (window::resized()) {
    m_base->recreate_swapchain();
    m_present_texture.destroy_descriptor_set_for_imgui();
    m_present_texture.window_resize(window::size().x, window::size().y);
    m_present_texture.prepare_descriptor_set_for_imgui();
    m_render_texture.window_resize(window::size().x, window::size().y);
    m_depth_texture.window_resize(window::size().x, window::size().y);
  }
}
void vk_renderer::add_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
  if (m_vertex_arrays.contains(name.data())) {
    return;
  }
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
void vk_renderer::draw2d(std::shared_ptr<struct drawable> drawObject) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = drawObject;
  add_texture(drawObject->binding_texture);
  register_vk_drawable(t, "2D");
}
void vk_renderer::drawui(std::shared_ptr<struct drawable> drawObject) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = drawObject;
  add_texture(drawObject->binding_texture);
  register_vk_drawable(t, "UI");
}

void vk_renderer::draw3d(std::shared_ptr<struct drawable> drawObject) {
  auto t = std::make_shared<vk_drawable>();
  t->drawable_obj = drawObject;
  add_texture(drawObject->binding_texture);
  register_vk_drawable(t, "3D");
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
  pipeline.initialize(m_pipeline_layout_instance, m_render_texture.render_pass,
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

void vk_renderer::prepare() {
  prepare_descriptor_set_layout();
  prepare_descriptor_pool();

  m_sampler = create_sampler();

  m_pipeline_layout_depth_instance.initialize(
      m_base->get_vk_device(), &m_descriptor_set_layout_for_depth,
      m_base->mSwapchain->GetSurfaceExtent());
  m_pipeline_layout_depth_instance.prepare(m_base->get_vk_device());
  m_pipeline_layout_depth.initialize(
      m_base->get_vk_device(), &m_descriptor_set_layout_for_depth,
      m_base->mSwapchain->GetSurfaceExtent(), false);
  m_pipeline_layout_depth.prepare(m_base->get_vk_device());

  m_pipeline_layout_instance.initialize(m_base->get_vk_device(),
                                        &m_descriptor_set_layout,
                                        m_base->mSwapchain->GetSurfaceExtent());
  m_pipeline_layout_instance.prepare(m_base->get_vk_device());
  m_pipeline_layout_normal.initialize(
      m_base->get_vk_device(), &m_descriptor_set_layout,
      m_base->mSwapchain->GetSurfaceExtent(), false);
  m_pipeline_layout_normal.prepare(m_base->get_vk_device());
  m_present_texture.prepare(window::size().x, window::size().y);
  m_render_texture.set_MSAA(true);
  m_render_texture.prepare(window::size().x, window::size().y);
  m_depth_texture.prepare(scene::size().x, scene::size().y);

  vk_pipeline_builder pipeline_builder(
      m_base->get_vk_device(), m_pipeline_layout_instance,
      m_pipeline_layout_normal, m_pipeline_layout_depth_instance,
      m_pipeline_layout_depth, m_render_texture, m_present_texture,
      m_depth_texture, m_base->m_renderPass);
  // render texture pipeline
  pipeline_builder.present_texture_pipeline(m_present_texture.pipeline);
  pipeline_builder.render_texture_pipeline(m_render_texture.pipeline);
  vk_pipeline pipeline_skybox;
  vk_pipeline pipeline_opaque;
  vk_pipeline pipeline_alpha;
  vk_pipeline pipeline_2d;
  vk_pipeline pipeline_instancing_opaque;
  vk_pipeline pipeline_instancing_alpha;
  vk_pipeline pipeline_instancing_2d;
  vk_pipeline pipeline_depth;
  vk_pipeline pipeline_depth_instancing;
  vk_pipeline pipeline_ui;

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
  // depth instancing pipeline
  pipeline_builder.depth_instancing(pipeline_depth_instancing);
  // UI pipeline
  pipeline_builder.ui(pipeline_ui);
  m_pipelines["skybox"] = pipeline_skybox;
  m_pipelines["opaque"] = pipeline_opaque;
  m_pipelines["alpha"] = pipeline_alpha;
  m_pipelines["2d"] = pipeline_2d;
  m_pipelines["instancing_opaque"] = pipeline_instancing_opaque;
  m_pipelines["instancing_alpha"] = pipeline_instancing_alpha;
  m_pipelines["instancing_2d"] = pipeline_instancing_2d;
  m_pipelines["depth"] = pipeline_depth;
  m_pipelines["depth_instancing"] = pipeline_depth_instancing;
  m_pipelines["ui"] = pipeline_ui;

  prepare_imgui();
  m_skybox = std::make_shared<vk_drawable>();
  m_skybox->drawable_obj = std::make_shared<drawable>();
  m_skybox->drawable_obj->vertexIndex = "BOX";
}
void vk_renderer::cleanup() {
  VkDevice device = m_base->get_vk_device();
  vkDeviceWaitIdle(device);
  destroy_vk_drawable(m_skybox);
  for (auto &sprite : m_draw_object_3d)
    destroy_vk_drawable(sprite);
  m_draw_object_3d.clear();
  for (auto &sprite : m_draw_object_2d)
    destroy_vk_drawable(sprite);
  m_draw_object_2d.clear();
  for (auto &sprite : m_draw_object_ui)
    destroy_vk_drawable(sprite);
  m_draw_object_ui.clear();
  m_present_texture.prepare_descriptor_set_for_imgui();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  m_present_texture.clear();
  m_render_texture.clear();
  m_depth_texture.clear();

  for (auto &i : m_image_object) {
    destroy_image(i.second);
  }
  vkutil::destroy_vulkan_object<VkSampler>(device, m_sampler,
                                           &vkDestroySampler);
  m_pipeline_layout_depth_instance.cleanup(device);
  m_pipeline_layout_depth.cleanup(device);
  m_pipeline_layout_instance.cleanup(device);
  m_pipeline_layout_normal.cleanup(device);
  for (auto &i : m_pipelines) {
    i.second.Cleanup(device);
  }
  for (auto &i : m_user_pipelines) {
    i.second.Cleanup(device);
  }
  for (auto &i : m_vertex_arrays) {
    destroy_buffer(i.second.vertexBuffer);
    destroy_buffer(i.second.indexBuffer);
  }
  m_vertex_arrays.clear();
  vkutil::destroy_vulkan_object<VkDescriptorPool>(device, m_descriptor_pool,
                                                  &vkDestroyDescriptorPool);
  vkutil::destroy_vulkan_object<VkDescriptorSetLayout>(
      device, m_descriptor_set_layout, &vkDestroyDescriptorSetLayout);
  vkutil::destroy_vulkan_object<VkDescriptorSetLayout>(
      device, m_descriptor_set_layout_for_depth, &vkDestroyDescriptorSetLayout);
}

void vk_renderer::make_command(VkCommandBuffer command) {
  draw_skybox(command);
  draw3d(command);
  draw2d(command);
}
void vk_renderer::draw_skybox(VkCommandBuffer command) {
  m_pipelines["skybox"].Bind(command);
  auto &va = m_vertex_arrays["BOX"];
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(command, 0, 1, &va.vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(command, va.indexBuffer.buffer, offset,
                       VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline_layout_normal.GetLayout(), 0, 1,
                          &m_skybox->descriptor_sets[m_base->m_imageIndex], 0,
                          nullptr);
  auto allocation = m_skybox->uniformBuffers[m_base->m_imageIndex].allocation;
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
  write_memory(allocation, &param, sizeof(vk_shader_parameter));
  vkCmdDrawIndexed(command, va.indexCount, 1, 0, 0, 0);
}

void vk_renderer::draw3d(VkCommandBuffer command, bool is_change_pipeline) {
  VkDeviceSize offset = 0;
  for (auto &sprite : m_draw_object_3d) {
    if (sprite->drawable_obj->size() > 0) {
      if (is_change_pipeline) {
        m_pipelines["instancing_opaque"].Bind(command);
      }
      vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipeline_layout_instance.GetLayout(), 0, 1,
                              &sprite->descriptor_sets[m_base->m_imageIndex], 0,
                              nullptr);
      auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
      vk_shader_parameter param;
      param.param = sprite->drawable_obj->param;
      param.param.light_proj = renderer::light_projection;
      param.param.light_view = renderer::light_view;
      auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
      write_memory(allocation, &param, sizeof(vk_shader_parameter));
      write_memory(allocation, ptr,
                   sprite->drawable_obj->shade.get_parameter_size(),
                   sizeof(vk_shader_parameter));
      std::string index = sprite->drawable_obj->vertexIndex;
      VkBuffer buffers[] = {m_vertex_arrays[index].vertexBuffer.buffer,
                            sprite->instance_buffer.buffer};
      VkDeviceSize offsets[] = {0, 0};
      vkCmdBindVertexBuffers(command, 0, 2, buffers, offsets);
      vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer,
                           0, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount,
                       sprite->drawable_obj->data.size(), 0, 0, 0);
    } else {

      if (is_change_pipeline) {
        if (sprite->drawable_obj->shade.vertex_shader() == "default" &&
            sprite->drawable_obj->shade.fragment_shader() == "default")
          m_pipelines["opaque"].Bind(command);
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
      vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipeline_layout_normal.GetLayout(), 0, 1,
                              &sprite->descriptor_sets[m_base->m_imageIndex], 0,
                              nullptr);
      auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
      vk_shader_parameter param;
      param.param = sprite->drawable_obj->param;
      param.param.light_proj = renderer::light_projection;
      param.param.light_view = renderer::light_view;
      auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
      write_memory(allocation, &param, sizeof(vk_shader_parameter));
      write_memory(allocation, ptr,
                   sprite->drawable_obj->shade.get_parameter_size(),
                   sizeof(vk_shader_parameter));
      vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount, 1, 0, 0, 0);
    }
  }
}

void vk_renderer::draw2d(VkCommandBuffer command) {
  VkDeviceSize offset = 0;
  for (auto &sprite : m_draw_object_2d) {
    if (sprite->drawable_obj->size() > 0) {
      m_pipelines["instancing_2d"].Bind(command);
      vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipeline_layout_instance.GetLayout(), 0, 1,
                              &sprite->descriptor_sets[m_base->m_imageIndex], 0,
                              nullptr);
      auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
      auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
      write_memory(allocation, &sprite->drawable_obj->param,
                   sizeof(vk_shader_parameter));
      write_memory(allocation, ptr,
                   sprite->drawable_obj->shade.get_parameter_size(),
                   sizeof(vk_shader_parameter));
      std::string index = sprite->drawable_obj->vertexIndex;
      VkBuffer buffers[] = {m_vertex_arrays[index].vertexBuffer.buffer,
                            sprite->instance_buffer.buffer};
      VkDeviceSize offsets[] = {0, 0};
      vkCmdBindVertexBuffers(command, 0, 2, buffers, offsets);
      vkCmdBindIndexBuffer(command, m_vertex_arrays[index].indexBuffer.buffer,
                           0, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(command, m_vertex_arrays[index].indexCount,
                       sprite->drawable_obj->data.size(), 0, 0, 0);
    } else {
      if (sprite->drawable_obj->shade.vertex_shader() == "default" &&
          sprite->drawable_obj->shade.fragment_shader() == "default")
        m_pipelines["2d"].Bind(command);
      else {
        for (auto &i : m_user_pipelines) {
          if (i.first == sprite->drawable_obj->shade)
            i.second.Bind(command);
        }
      }
      vkCmdBindVertexBuffers(command, 0, 1,
                             &m_vertex_arrays[sprite->drawable_obj->vertexIndex]
                                  .vertexBuffer.buffer,
                             &offset);
      vkCmdBindIndexBuffer(
          command,
          m_vertex_arrays[sprite->drawable_obj->vertexIndex].indexBuffer.buffer,
          offset, VK_INDEX_TYPE_UINT32);

      // Set descriptors
      vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipeline_layout_normal.GetLayout(), 0, 1,
                              &sprite->descriptor_sets[m_base->m_imageIndex], 0,
                              nullptr);
      auto allocation = sprite->uniformBuffers[m_base->m_imageIndex].allocation;
      auto *ptr = sprite->drawable_obj->shade.get_parameter().get();
      write_memory(allocation, &sprite->drawable_obj->param,
                   sizeof(vk_shader_parameter));
      write_memory(allocation, ptr,
                   sprite->drawable_obj->shade.get_parameter_size(),
                   sizeof(vk_shader_parameter));

      vkCmdDrawIndexed(
          command,
          m_vertex_arrays[sprite->drawable_obj->vertexIndex].indexCount, 1, 0,
          0, 0);
    }
  }
}
void vk_renderer::drawui(VkCommandBuffer command) {
  VkDeviceSize offset = 0;
  for (auto &sprite : m_draw_object_ui) {
    m_pipelines["ui"].Bind(command);
    vkCmdBindVertexBuffers(
        command, 0, 1,
        &m_vertex_arrays[sprite->drawable_obj->vertexIndex].vertexBuffer.buffer,
        &offset);
    vkCmdBindIndexBuffer(
        command,
        m_vertex_arrays[sprite->drawable_obj->vertexIndex].indexBuffer.buffer,
        offset, VK_INDEX_TYPE_UINT32);

    // Set descriptors
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout_normal.GetLayout(), 0, 1,
                            &sprite->descriptor_sets[m_base->m_imageIndex], 0,
                            nullptr);
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
  vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipeline_layout_normal.GetLayout(), 0, 1,
                          &sprite.descriptor_sets[m_base->m_imageIndex], 0,
                          nullptr);
  auto allocation = sprite.uniformBuffers[m_base->m_imageIndex].allocation;

  sprite.drawable_obj->param.proj = matrix4::identity;
  sprite.drawable_obj->param.view = matrix4::identity;
  sprite.drawable_obj->param.world = matrix4::identity;
  sprite.drawable_obj->param.user = renderer::render_texture_user_data;
  write_memory(allocation, &sprite.drawable_obj->param,
               sizeof(vk_shader_parameter));

  vkCmdDrawIndexed(command, m_vertex_arrays["SPRITE"].indexCount, 1, 0, 0, 0);
}

void vk_renderer::write_memory(VmaAllocation allocation, const void *data,
                               std::size_t size, std::size_t offset) {
  if (size <= 0) {
    return;
  }
  void *p = nullptr;
  char *pc = nullptr;
  vmaMapMemory(allocator, allocation, &p);
  pc = reinterpret_cast<char *>(p);
  pc += offset;
  p = reinterpret_cast<void *>(pc);
  std::memcpy(p, data, size);
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
  m_present_texture.prepare_descriptor_set_for_imgui();
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
  VkDescriptorSetLayoutBinding ubo{}, tex{}, depth{};
  ubo.binding = 0;
  ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  ubo.descriptorCount = 1;
  bindings.push_back(ubo);

  tex.binding = 1;
  tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  tex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  tex.descriptorCount = 1;
  bindings.push_back(tex);

  depth.binding = 2;
  depth.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  depth.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  depth.descriptorCount = 1;
  bindings.push_back(depth);

  VkDescriptorSetLayoutCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ci.bindingCount = uint32_t(bindings.size());
  ci.pBindings = bindings.data();
  vkCreateDescriptorSetLayout(m_base->get_vk_device(), &ci, nullptr,
                              &m_descriptor_set_layout);

  bindings.clear();
  bindings.push_back(ubo);
  ci.bindingCount = uint32_t(bindings.size());
  ci.pBindings = bindings.data();
  vkCreateDescriptorSetLayout(m_base->get_vk_device(), &ci, nullptr,
                              &m_descriptor_set_layout_for_depth);
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

  {
    ai.descriptorSetCount = 2;
    std::array<VkDescriptorSetLayout, 2> layouts;
    for (auto &i : layouts) {
      i = m_descriptor_set_layout;
    }
    ai.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(m_base->get_vk_device(), &ai,
                             sprite->descriptor_sets.data());
    // Write to descriptor set.
    for (int i = 0; i < m_base->mSwapchain->GetImageCount(); i++) {
      std::vector<VkWriteDescriptorSet> writeSets;
      VkDescriptorBufferInfo descUBO{};
      descUBO.buffer = sprite->uniformBuffers[i].buffer;
      descUBO.offset = 0;
      descUBO.range = VK_WHOLE_SIZE;

      VkDescriptorImageInfo descImage;
      VkDescriptorImageInfo desc_depth;

      descImage.imageView =
          m_image_object[sprite->drawable_obj->binding_texture.handle].view;
      descImage.sampler = m_sampler;
      descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      desc_depth.imageView = m_depth_texture.color_target.view;
      desc_depth.sampler = m_sampler;
      desc_depth.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      VkWriteDescriptorSet ubo{};
      ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      ubo.dstBinding = 0;
      ubo.descriptorCount = 1;
      ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo.pBufferInfo = &descUBO;
      ubo.dstSet = sprite->descriptor_sets[i];
      writeSets.push_back(ubo);

      VkWriteDescriptorSet tex{};
      tex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      tex.dstBinding = 1;
      tex.descriptorCount = 1;
      tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      tex.pImageInfo = &descImage;
      tex.dstSet = sprite->descriptor_sets[i];
      writeSets.push_back(tex);

      VkWriteDescriptorSet depth{};
      depth.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      depth.dstBinding = 2;
      depth.descriptorCount = 1;
      depth.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      depth.pImageInfo = &desc_depth;
      depth.dstSet = sprite->descriptor_sets[i];
      writeSets.push_back(depth);
      vkUpdateDescriptorSets(m_base->get_vk_device(),
                             uint32_t(writeSets.size()), writeSets.data(), 0,
                             nullptr);
    }
  }
  {
    ai.descriptorSetCount = 2;
    std::array<VkDescriptorSetLayout, 2> layouts;
    for (auto &i : layouts) {
      i = m_descriptor_set_layout_for_depth;
    }
    ai.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(m_base->get_vk_device(), &ai,
                             sprite->descriptor_sets_for_depth.data());
    // Write to descriptor set.
    for (int i = 0; i < m_base->mSwapchain->GetImageCount(); i++) {
      std::vector<VkWriteDescriptorSet> writeSets;
      VkDescriptorBufferInfo descUBO{};
      descUBO.buffer = sprite->uniformBuffers[i].buffer;
      descUBO.offset = 0;
      descUBO.range = VK_WHOLE_SIZE;
      VkWriteDescriptorSet ubo{};
      ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      ubo.dstBinding = 0;
      ubo.descriptorCount = 1;
      ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo.pBufferInfo = &descUBO;
      ubo.dstSet = sprite->descriptor_sets_for_depth[i];
      writeSets.push_back(ubo);
      vkUpdateDescriptorSets(m_base->get_vk_device(),
                             uint32_t(writeSets.size()), writeSets.data(), 0,
                             nullptr);
    }
  }
}
vk_buffer vk_renderer::create_buffer(uint32_t size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags flags,
                                     VmaMemoryUsage vma_usage) {

  vk_buffer obj;
  VkBufferCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  ci.usage = usage;
  ci.size = size;
  VmaAllocationCreateInfo buffer_alloc_info = {};
  buffer_alloc_info.usage = vma_usage;
  vmaCreateBuffer(allocator, &ci, &buffer_alloc_info, &obj.buffer,
                  &obj.allocation, nullptr);
  return obj;
}

VkSampler vk_renderer::create_sampler() {
  VkSampler sampler;
  VkSamplerCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  ci.minFilter = VK_FILTER_NEAREST;
  ci.magFilter = VK_FILTER_NEAREST;
  ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  ci.maxAnisotropy = 1.0f;
  ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
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
  if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  imb.image = image;

  VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
  VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  switch (oldLayout) {
  case VK_IMAGE_LAYOUT_UNDEFINED:
    imb.srcAccessMask = 0;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  default:
    break;
  }
  switch (newLayout) {
  case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    break;
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    break;
  default:
    break;
  }

  vkCmdPipelineBarrier(command, srcStage, dstStage, 0,
                       0, // memoryBarrierCount
                       nullptr,
                       0, // bufferMemoryBarrierCount
                       nullptr,
                       1, // imageMemoryBarrierCount
                       &imb);
}

void vk_renderer::destroy_buffer(vk_buffer &bufferObj) {
  if (bufferObj.buffer == VK_NULL_HANDLE ||
      bufferObj.allocation == VK_NULL_HANDLE) {
    return;
  }
  vmaDestroyBuffer(allocator, bufferObj.buffer, bufferObj.allocation);
  bufferObj.buffer = VK_NULL_HANDLE;
  bufferObj.allocation = VK_NULL_HANDLE;
}

void vk_renderer::destroy_image(vk_image &imageObj) {
  if (imageObj.view == VK_NULL_HANDLE | imageObj.image == VK_NULL_HANDLE) {
    return;
  }
  vkDestroyImageView(m_base->get_vk_device(), imageObj.view, nullptr);
  vmaDestroyImage(allocator, imageObj.image, imageObj.allocation);
  imageObj.image = VK_NULL_HANDLE;
  imageObj.view = VK_NULL_HANDLE;
  imageObj.allocation = VK_NULL_HANDLE;
}

void vk_renderer::create_image_object(const handle_t &handle) {
  SDL_Surface *surfptr = reinterpret_cast<SDL_Surface *>(handle);
  ::SDL_Surface &surf = *surfptr;
  ::SDL_LockSurface(&surf);
  auto *formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
  formatbuf->BytesPerPixel = 4;
  auto *imagedata = ::SDL_ConvertSurface(&surf, formatbuf, 0);
  ::SDL_UnlockSurface(&surf);
  vk_buffer stagingBuffer;
  vk_image image;
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
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    vmaCreateImage(allocator, &ci, &alloc_info, &image.image, &image.allocation,
                   nullptr);
  }

  {
    uint32_t imageSize = imagedata->h * imagedata->w * 4;
    stagingBuffer = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                  VMA_MEMORY_USAGE_GPU_TO_CPU);
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
  set_image_memory_barrier(command, image.image, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vkCmdCopyBufferToImage(command, stagingBuffer.buffer, image.image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

  set_image_memory_barrier(command, image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  vkEndCommandBuffer(command);

  VkSubmitInfo submitInfo{};
  vkResetFences(m_base->get_vk_device(), 1,
                &m_base->m_fences[m_base->m_imageIndex]);
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &command;
  vkQueueSubmit(m_base->get_vk_queue(), 1, &submitInfo,
                m_base->m_fences[m_base->m_imageIndex]);
  vkQueueWaitIdle(m_base->get_vk_queue());

  // Create view for texture reference
  VkImageViewCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ci.image = image.image;
  ci.format = VK_FORMAT_R8G8B8A8_UNORM;
  ci.components = {
      .r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
      .g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
      .b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
      .a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A,
  };

  ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  vkCreateImageView(m_base->get_vk_device(), &ci, nullptr, &image.view);
  vkDeviceWaitIdle(m_base->get_vk_device());
  vkFreeCommandBuffers(m_base->get_vk_device(), m_base->m_commandPool, 1,
                       &command);
  destroy_buffer(stagingBuffer);
  SDL_FreeFormat(formatbuf);
  SDL_FreeSurface(imagedata);
  m_image_object.emplace(handle, image);
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
void vk_renderer::register_vk_drawable(
    std::shared_ptr<vk_drawable> _vk_drawable, std::string_view type) {
  auto drawObject = _vk_drawable->drawable_obj;
  if (drawObject->size() > 0) {
    _vk_drawable->instance_buffer = create_buffer(
        drawObject->size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,

        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VMA_MEMORY_USAGE_GPU_TO_CPU);
    write_memory(_vk_drawable->instance_buffer.allocation,
                 drawObject->data.data(), drawObject->size());
  }
  if (type == "3D") {
    m_draw_object_3d.push_back(_vk_drawable);
    for (auto &v : _vk_drawable->uniformBuffers) {
      VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      auto shader_param_size =
          _vk_drawable->drawable_obj->shade.get_parameter_size();
      v = create_buffer(sizeof(vk_shader_parameter) + shader_param_size,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags,
                        VMA_MEMORY_USAGE_GPU_TO_CPU);
    }
    prepare_descriptor_set(_vk_drawable);
  } else if (type == "2D") {
    auto iter = m_draw_object_2d.begin();
    for (; iter != m_draw_object_2d.end(); ++iter) {
      if (_vk_drawable->drawable_obj->drawOrder <
          (*iter)->drawable_obj->drawOrder) {
        break;
      }
    }

    // Inserts element before position of iterator
    m_draw_object_2d.insert(iter, _vk_drawable);
    for (auto &v : _vk_drawable->uniformBuffers) {
      VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      v = create_buffer(
          sizeof(vk_shader_parameter) +
              _vk_drawable->drawable_obj->shade.get_parameter_size(),
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
    }
    prepare_descriptor_set(_vk_drawable);
  } else if (type == "UI") {
    auto iter = m_draw_object_ui.begin();
    for (; iter != m_draw_object_ui.end(); ++iter) {
      if (_vk_drawable->drawable_obj->drawOrder <
          (*iter)->drawable_obj->drawOrder) {
        break;
      }
    }

    // Inserts element before position of iterator
    m_draw_object_ui.insert(iter, _vk_drawable);
    for (auto &v : _vk_drawable->uniformBuffers) {
      VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      v = create_buffer(
          sizeof(vk_shader_parameter) +
              _vk_drawable->drawable_obj->shade.get_parameter_size(),
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
    }
    prepare_descriptor_set(_vk_drawable);
  }
}
void vk_renderer::destroy_vk_drawable(std::shared_ptr<vk_drawable> texture) {
  auto device = m_base->get_vk_device();
  if (texture->descriptor_sets[0] != VK_NULL_HANDLE) {
    vkFreeDescriptorSets(device, m_descriptor_pool,
                         static_cast<uint32_t>(texture->descriptor_sets.size()),
                         texture->descriptor_sets.data());
  }
  texture->descriptor_sets[0] = VK_NULL_HANDLE;
  texture->descriptor_sets[1] = VK_NULL_HANDLE;
  if (texture->descriptor_sets_for_depth[0] != VK_NULL_HANDLE) {
    vkFreeDescriptorSets(
        device, m_descriptor_pool,
        static_cast<uint32_t>(texture->descriptor_sets_for_depth.size()),
        texture->descriptor_sets_for_depth.data());
  }
  texture->descriptor_sets_for_depth[0] = VK_NULL_HANDLE;
  texture->descriptor_sets_for_depth[1] = VK_NULL_HANDLE;
  for (auto &i : texture->uniformBuffers) {
    destroy_buffer(i);
  }
  if (texture->drawable_obj->size() > 0) {
    destroy_buffer(texture->instance_buffer);
  }
}
} // namespace sinen
#endif
