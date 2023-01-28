#ifndef VK_PIPELINE_BUILDER_HPP
#define VK_PIPELINE_BUILDER_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include "vk_pipeline.hpp"
#include "vk_render_texture.hpp"

namespace sinen {
class vk_pipeline_builder {
public:
  vk_pipeline_builder(VkDevice device,
                      vk_pipeline_layout &pipeline_layout_instance,
                      vk_pipeline_layout &pipeline_layout_normal,
                      vk_render_texture &render_texture,
                      VkRenderPass &render_pass)
      : device(device), pipeline_layout_instance(pipeline_layout_instance),
        pipeline_layout_normal(pipeline_layout_normal),
        render_texture(render_texture), render_pass(render_pass) {}
  void skybox(vk_pipeline &pipeline);
  void opaque(vk_pipeline &pipeline);
  void alpha(vk_pipeline &pipeline);
  void alpha_2d(vk_pipeline &pipeline);
  void instancing_opaque(vk_pipeline &pipeline);
  void instancing_alpha(vk_pipeline &pipeline);
  void instancing_alpha_2d(vk_pipeline &pipeline);
  void ui(vk_pipeline &pipeline);
  void render_texture_pipeline(vk_pipeline &pipeline);

private:
  VkDevice device;
  vk_pipeline_layout &pipeline_layout_instance;
  vk_pipeline_layout &pipeline_layout_normal;
  vk_render_texture &render_texture;
  VkRenderPass &render_pass;
};

} // namespace sinen
#endif // !defined(EMSCRIPTEN) && !defined(ANDROID)
#endif // VK_PIPELINE_BUILDER_HPP