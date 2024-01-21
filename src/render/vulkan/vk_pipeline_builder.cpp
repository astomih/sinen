#include "vk_pipeline_builder.hpp"
#include "vk_shader.hpp"

namespace sinen {

void vk_pipeline_builder::skybox(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_normal, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);

  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::opaque(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderOpaque.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_normal, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::alpha(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_normal, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_TRUE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::alpha_2d(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_normal, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);

  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::instancing_opaque(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader_instance.vert.spv",
                      VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderOpaque.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_instance, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::instancing_alpha(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader_instance.vert.spv",
                      VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_instance, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_TRUE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::instancing_alpha_2d(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader_instance.vert.spv",
                      VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_instance, render_texture.render_pass,
                      shaderStages);
  if (is_MSAA)
    pipeline.set_sample_count(VK_SAMPLE_COUNT_4_BIT);
  else
    pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::depth(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "depth.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "depth.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_depth, depth_texture.render_pass,
                      shaderStages);
  pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_TRUE);
  pipeline.set_depth_write(VK_TRUE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::depth_instancing(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "depth_instance.vert.spv",
                      VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "depth.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_depth_instance, depth_texture.render_pass,
                      shaderStages);
  pipeline.set_sample_count(VK_SAMPLE_COUNT_1_BIT);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_TRUE);
  pipeline.set_depth_write(VK_TRUE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::ui(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "shaderAlpha.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(pipeline_layout_normal, render_pass, shaderStages);

  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::render_texture_pipeline(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "render_texture.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(render_texture.m_pipeline_layout,
                      present_texture.render_pass, shaderStages);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
void vk_pipeline_builder::present_texture_pipeline(vk_pipeline &pipeline) {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      vk_shader::load(device, "shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      vk_shader::load(device, "render_texture.frag.spv",
                      VK_SHADER_STAGE_FRAGMENT_BIT)};
  pipeline.initialize(present_texture.m_pipeline_layout, render_pass,
                      shaderStages);
  pipeline.color_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.alpha_blend_factor(VK_BLEND_FACTOR_SRC_ALPHA,
                              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
  pipeline.set_depth_test(VK_FALSE);
  pipeline.set_depth_write(VK_FALSE);
  pipeline.prepare(device);
  vk_shader::clean(device, shaderStages);
}
} // namespace sinen