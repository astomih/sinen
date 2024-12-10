#include "render_system.hpp"
#include "../vertex/default_model_creator.hpp"
#include "vulkan/vk_renderer.hpp"
#include <io/asset_type.hpp>
#include <io/data_stream.hpp>
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

#include <imgui.h>

namespace sinen {
Color RendererImpl::clearColor = Palette::black();
vk_renderer RendererImpl::m_vk_renderer;
// Renderer
bool RendererImpl::showImGui = false;
std::list<std::function<void()>> RendererImpl::m_imgui_function;
Texture RendererImpl::m_skybox_texture;
std::vector<std::shared_ptr<Drawable>> RendererImpl::m_drawable_2d;
std::vector<std::shared_ptr<Drawable>> RendererImpl::m_drawable_3d;
void RendererImpl::unload_data() {}
void RendererImpl::initialize() {
  m_vk_renderer.initialize();
  setup_shapes();
}

void RendererImpl::shutdown() { m_vk_renderer.shutdown(); }

void RendererImpl::render() {
  for (auto &d : m_drawable_3d) {
    draw3d(d);
  }
  for (auto &d : m_drawable_2d) {
    draw2d(d);
  }
  m_vk_renderer.render();
}
void RendererImpl::draw2d(std::shared_ptr<Drawable> drawObject) {
  m_vk_renderer.draw2d(drawObject);
}
void RendererImpl::drawui(std::shared_ptr<Drawable> drawObject) {
  m_vk_renderer.drawui(drawObject);
}

void RendererImpl::draw3d(std::shared_ptr<Drawable> drawObject) {
  m_vk_renderer.draw3d(drawObject);
}
void RendererImpl::add_queue_2d(const std::shared_ptr<Drawable> draw_object) {
  m_drawable_2d.push_back(draw_object);
}
void RendererImpl::add_queue_3d(const std::shared_ptr<Drawable> draw_object) {
  m_drawable_3d.push_back(draw_object);
}
void RendererImpl::remove_queue_2d(
    const std::shared_ptr<Drawable> draw_object) {
  m_drawable_2d.erase(
      std::remove(m_drawable_2d.begin(), m_drawable_2d.end(), draw_object),
      m_drawable_2d.end());
}
void RendererImpl::remove_queue_3d(
    const std::shared_ptr<Drawable> draw_object) {
  m_drawable_3d.erase(
      std::remove(m_drawable_3d.begin(), m_drawable_3d.end(), draw_object),
      m_drawable_3d.end());
}
void RendererImpl::add_vertex_array(const VertexArray &vArray,
                                     std::string_view name) {
  m_vk_renderer.add_vertex_array(vArray, name);
}

void RendererImpl::update_vertex_array(const VertexArray &vArray,
                                        std::string_view name) {
  m_vk_renderer.update_vertex_array(vArray, name);
}
void RendererImpl::add_model(const Model &m) { m_vk_renderer.add_model(m); }
void RendererImpl::update_model(const Model &m) {
  m_vk_renderer.update_model(m);
}

void RendererImpl::load_shader(const Shader &shaderInfo) {
  m_vk_renderer.load_shader(shaderInfo);
}

void RendererImpl::unload_shader(const Shader &shaderInfo) {
  m_vk_renderer.unload_shader(shaderInfo);
}
void *RendererImpl::get_texture_id() { return m_vk_renderer.get_texture_id(); }

void RendererImpl::setup_shapes() {
  add_vertex_array(create_box_vertices(), VertexDefaultShapes::box);
  add_vertex_array(create_sprite_vertices(), VertexDefaultShapes::sprite);
}

void RendererImpl::prepare_imgui() {
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontFromFileTTF(
      DataStream::convert_file_path(AssetType::Font,
                                     "mplus/mplus-1p-medium.ttf")
          .data(),
      18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
}
} // namespace sinen
