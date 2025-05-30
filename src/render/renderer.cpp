#include "render_system.hpp"
#include <asset/asset.hpp>
#include <render/renderer.hpp>

namespace sinen {
bool Renderer::offscreen_rendering = false;
glm::mat4 Renderer::render_texture_user_data;

void Renderer::unload_data() { RendererSystem::unload_data(); }

void Renderer::render() { RendererSystem::render(); }

void Renderer::draw2d(const std::shared_ptr<Drawable> draw_object) {
  RendererSystem::draw2d(draw_object);
}
void Renderer::draw3d(const std::shared_ptr<Drawable> draw_object) {
  RendererSystem::draw3d(draw_object);
}
void Renderer::set_clear_color(const Color &color) {
  RendererSystem::set_clear_color(color);
}
Color Renderer::clear_color() { return RendererSystem::get_clear_color(); }
void Renderer::toggle_show_imgui() { RendererSystem::toggle_show_imgui(); }
bool Renderer::is_show_imgui() { return RendererSystem::is_show_imgui(); }
std::list<std::function<void()>> &Renderer::get_imgui_function() {
  return RendererSystem::get_imgui_function();
}

void Renderer::add_imgui_function(std::function<void()> function) {
  RendererSystem::get_imgui_function().push_back(function);
}
void *Renderer::get_texture_id() { return RendererSystem::get_texture_id(); }

void Renderer::begin_pipeline3d(const RenderPipeline3D &pipeline) {
  RendererSystem::begin_pipeline3d(pipeline);
}
void Renderer::end_pipeline3d() { RendererSystem::end_pipeline3d(); }
void Renderer::begin_pipeline2d(const RenderPipeline2D &pipeline) {
  RendererSystem::begin_pipeline2d(pipeline);
}
void Renderer::end_pipeline2d() { RendererSystem::end_pipeline2d(); }
void Renderer::set_uniform_data(uint32_t slot, const UniformData &data) {
  RendererSystem::set_uniform_data(slot, data);
}
void Renderer::begin_render_texture2d(const RenderTexture &texture) {
  RendererSystem::begin_render_texture2d(texture);
}
void Renderer::begin_render_texture3d(const RenderTexture &texture) {
  RendererSystem::begin_render_texture3d(texture);
}
void Renderer::end_render_texture(const RenderTexture &texture, Texture &out) {
  RendererSystem::end_render_texture(texture, out);
}
} // namespace sinen
