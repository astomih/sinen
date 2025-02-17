#include "render_system.hpp"
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
bool Renderer::offscreen_rendering = false;
matrix4 Renderer::render_texture_user_data;
matrix4 Renderer::light_view;
matrix4 Renderer::light_projection;

void Renderer::unload_data() { RendererImpl::unload_data(); }

void Renderer::render() { RendererImpl::render(); }

void Renderer::draw2d(const std::shared_ptr<Drawable> draw_object) {
  RendererImpl::draw2d(draw_object);
}
void Renderer::draw3d(const std::shared_ptr<Drawable> draw_object) {
  RendererImpl::draw3d(draw_object);
}
void Renderer::add_vertex_array(const VertexArray &vArray,
                                std::string_view name) {
  RendererImpl::add_vertex_array(vArray, name);
}
void Renderer::update_vertex_array(const VertexArray &vArray,
                                   std::string_view name) {
  RendererImpl::update_vertex_array(vArray, name);
}
void Renderer::add_model(const Model &_model) {
  RendererImpl::add_model(_model);
}
void Renderer::update_model(const Model &_model) {
  RendererImpl::update_model(_model);
}

void Renderer::set_clear_color(const Color &color) {
  RendererImpl::set_clear_color(color);
}

Color Renderer::clear_color() { return RendererImpl::get_clear_color(); }

void Renderer::set_skybox(Texture _skybox_texture) {
  RendererImpl::set_skybox_texture(_skybox_texture);
}

Texture Renderer::skybox() { return RendererImpl::get_skybox_texture(); }

void Renderer::toggle_show_imgui() { RendererImpl::toggle_show_imgui(); }
bool Renderer::is_show_imgui() { return RendererImpl::is_show_imgui(); }

std::list<std::function<void()>> &Renderer::get_imgui_function() {
  return RendererImpl::get_imgui_function();
}

void Renderer::add_imgui_function(std::function<void()> function) {
  RendererImpl::get_imgui_function().push_back(function);
}
void *Renderer::get_texture_id() { return RendererImpl::get_texture_id(); }
void Renderer::set_light_look_at(const Vector3 &position, const Vector3 &target,
                                 const Vector3 &up) {
  light_view = matrix4::lookat(position, target, up);
}
void Renderer::set_light_ortho(float left, float right, float bottom, float top,
                               float near, float far) {
  light_projection = matrix4::ortho(left, right, bottom, top, near, far);
}

} // namespace sinen
