#include "graphics_system.hpp"
#include <asset/asset.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
bool Graphics::offscreen_rendering = false;
glm::mat4 Graphics::render_texture_user_data;

void Graphics::unload_data() { GraphicsSystem::unload_data(); }

void Graphics::render() { GraphicsSystem::render(); }

void Graphics::draw2d(const std::shared_ptr<Drawable> draw_object) {
  GraphicsSystem::draw2d(draw_object);
}
void Graphics::draw3d(const std::shared_ptr<Drawable> draw_object) {
  GraphicsSystem::draw3d(draw_object);
}
void Graphics::SetClearColor(const Color &color) {
  GraphicsSystem::set_clear_color(color);
}
Color Graphics::GetClearColor() { return GraphicsSystem::get_clear_color(); }
void Graphics::toggle_show_imgui() { GraphicsSystem::toggle_show_imgui(); }
bool Graphics::is_show_imgui() { return GraphicsSystem::is_show_imgui(); }
std::list<std::function<void()>> &Graphics::get_imgui_function() {
  return GraphicsSystem::get_imgui_function();
}

void Graphics::add_imgui_function(std::function<void()> function) {
  GraphicsSystem::get_imgui_function().push_back(function);
}
void *Graphics::get_texture_id() { return GraphicsSystem::get_texture_id(); }

void Graphics::BindPipeline3D(const GraphicsPipeline3D &pipeline) {
  GraphicsSystem::bind_pipeline3d(pipeline);
}
void Graphics::BindDefaultPipeline3D() {
  GraphicsSystem::bind_default_pipeline3d();
}
void Graphics::BindPipeline2D(const GraphicsPipeline2D &pipeline) {
  GraphicsSystem::bind_pipeline2d(pipeline);
}
void Graphics::BindDefaultPipeline2D() {
  GraphicsSystem::bind_default_pipeline2d();
}
void Graphics::SetUniformData(uint32_t slot, const UniformData &data) {
  GraphicsSystem::set_uniform_data(slot, data);
}
void Graphics::BeginTarget2D(const RenderTexture &texture) {
  GraphicsSystem::begin_target2d(texture);
}
void Graphics::BeginTarget3D(const RenderTexture &texture) {
  GraphicsSystem::begin_target3d(texture);
}
void Graphics::EndTarget(const RenderTexture &texture, Texture &out) {
  GraphicsSystem::end_target(texture, out);
}
} // namespace sinen
