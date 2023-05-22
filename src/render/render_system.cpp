#include "render_system.hpp"
#include "../vertex/default_model_creator.h"
#include "vulkan/vk_renderer.hpp"
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
color render_system::clearColor = palette::black();
vk_renderer render_system::m_vk_renderer;
// Renderer
bool render_system::showImGui = false;
std::list<std::function<void()>> render_system::m_imgui_function;
texture render_system::m_skybox_texture;
std::vector<std::shared_ptr<drawable>> render_system::m_drawable_2d;
std::vector<std::shared_ptr<drawable>> render_system::m_drawable_3d;
void render_system::unload_data() {}
void render_system::initialize() {
  m_vk_renderer.initialize();
  setup_shapes();
}

void render_system::shutdown() { m_vk_renderer.shutdown(); }

void render_system::render() {
  for (auto &d : m_drawable_3d) {
    draw3d(d);
  }
  for (auto &d : m_drawable_2d) {
    draw2d(d);
  }
  m_vk_renderer.render();
}
void render_system::draw2d(std::shared_ptr<drawable> drawObject) {
  m_vk_renderer.draw2d(drawObject);
}
void render_system::drawui(std::shared_ptr<drawable> drawObject) {
  m_vk_renderer.drawui(drawObject);
}

void render_system::draw3d(std::shared_ptr<drawable> drawObject) {
  m_vk_renderer.draw3d(drawObject);
}
void render_system::add_queue_2d(const std::shared_ptr<drawable> draw_object) {
  m_drawable_2d.push_back(draw_object);
}
void render_system::add_queue_3d(const std::shared_ptr<drawable> draw_object) {
  m_drawable_3d.push_back(draw_object);
}
void render_system::remove_queue_2d(
    const std::shared_ptr<drawable> draw_object) {
  m_drawable_2d.erase(
      std::remove(m_drawable_2d.begin(), m_drawable_2d.end(), draw_object),
      m_drawable_2d.end());
}
void render_system::remove_queue_3d(
    const std::shared_ptr<drawable> draw_object) {
  m_drawable_3d.erase(
      std::remove(m_drawable_3d.begin(), m_drawable_3d.end(), draw_object),
      m_drawable_3d.end());
}
void render_system::add_vertex_array(const vertex_array &vArray,
                                     std::string_view name) {
  m_vk_renderer.add_vertex_array(vArray, name);
}

void render_system::update_vertex_array(const vertex_array &vArray,
                                        std::string_view name) {
  m_vk_renderer.update_vertex_array(vArray, name);
}
void render_system::add_model(const model &m) { m_vk_renderer.add_model(m); }
void render_system::update_model(const model &m) {
  m_vk_renderer.update_model(m);
}

void render_system::load_shader(const shader &shaderInfo) {
  m_vk_renderer.load_shader(shaderInfo);
}

void render_system::unload_shader(const shader &shaderInfo) {
  m_vk_renderer.unload_shader(shaderInfo);
}
void *render_system::get_texture_id() { return m_vk_renderer.get_texture_id(); }

void render_system::setup_shapes() {
  add_vertex_array(create_box_vertices(), vertex_default_shapes::box);
  add_vertex_array(create_sprite_vertices(), vertex_default_shapes::sprite);
}
} // namespace sinen
