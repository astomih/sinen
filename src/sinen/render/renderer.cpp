#include "render_system.hpp"
#include <math/vector3.hpp>
#include <model/vertex.hpp>
#include <model/vertex_array.hpp>
#include <render/renderer.hpp>

namespace sinen {
bool Renderer::offscreen_rendering = false;
matrix4 Renderer::render_texture_user_data;

void Renderer::unload_data() { RendererImpl::unload_data(); }

void Renderer::render() { RendererImpl::render(); }

void Renderer::draw2d(const std::shared_ptr<Drawable> draw_object) {
  RendererImpl::draw2d(draw_object);
}
void Renderer::draw3d(const std::shared_ptr<Drawable> draw_object) {
  RendererImpl::draw3d(draw_object);
}
void Renderer::set_clear_color(const Color &color) {
  RendererImpl::set_clear_color(color);
}
Color Renderer::clear_color() { return RendererImpl::get_clear_color(); }
void Renderer::toggle_show_imgui() { RendererImpl::toggle_show_imgui(); }
bool Renderer::is_show_imgui() { return RendererImpl::is_show_imgui(); }
std::list<std::function<void()>> &Renderer::get_imgui_function() {
  return RendererImpl::get_imgui_function();
}

void Renderer::add_imgui_function(std::function<void()> function) {
  RendererImpl::get_imgui_function().push_back(function);
}
void *Renderer::get_texture_id() { return RendererImpl::get_texture_id(); }

void Renderer::begin_pipeline3d(const RenderPipeline3D &pipeline) {
  RendererImpl::begin_pipeline3d(pipeline);
}
void Renderer::end_pipeline3d() { RendererImpl::end_pipeline3d(); }
void Renderer::begin_pipeline2d(const RenderPipeline2D &pipeline) {
  RendererImpl::begin_pipeline2d(pipeline);
}
void Renderer::end_pipeline2d() { RendererImpl::end_pipeline2d(); }
void Renderer::set_uniform_data(uint32_t slot, const UniformData &data) {
  RendererImpl::set_uniform_data(slot, data);
}
} // namespace sinen
