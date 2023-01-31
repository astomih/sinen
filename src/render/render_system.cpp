#include "render_system.hpp"
#include "../vertex/default_model_creator.h"
#include "opengl/gl_renderer.hpp"
#include "vulkan/vk_renderer.hpp"
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
color render_system::clearColor = palette::black();
std::unique_ptr<class gl_renderer> render_system::m_gl_renderer;
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
std::unique_ptr<class vk_renderer> render_system::m_vk_renderer;
#endif
// Renderer
graphics_api render_system::RendererAPI;
bool render_system::showImGui = false;
std::list<std::function<void()>> render_system::m_imgui_function;
texture render_system::m_skybox_texture;
std::vector<std::shared_ptr<drawable>> render_system::m_drawable_2d;
std::vector<std::shared_ptr<drawable>> render_system::m_drawable_3d;
std::vector<std::shared_ptr<instancing>> render_system::m_instancing;
pool_allocator<drawable, 2048> render_system::m_drawable_pool;
graphics_api render_system::get_graphics_api() { return RendererAPI; }
void render_system::unload_data() {}
void render_system::initialize(graphics_api api) {
  RendererAPI = api;
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer = std::make_unique<vk_renderer>();
    m_vk_renderer->initialize();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer = std::make_unique<gl_renderer>();
    m_gl_renderer->initialize();
  }
  setup_shapes();
}

void render_system::shutdown() {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->shutdown();
  }
#endif
  if (m_gl_renderer) {
    m_gl_renderer->shutdown();
  }
}

void render_system::render() {
  for (auto &d : m_drawable_3d) {
    draw3d(d);
  }
  for (auto &d : m_drawable_2d) {
    draw2d(d);
  }
  for (auto &i : m_instancing) {
    add_instancing(*i);
  }
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->render();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->render();
  }
}
void render_system::draw2d(std::shared_ptr<drawable> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw2d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->draw2d(drawObject);
  }
}
void render_system::drawui(std::shared_ptr<drawable> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->drawui(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->drawui(drawObject);
  }
}

void render_system::draw3d(std::shared_ptr<drawable> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw3d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->draw3d(drawObject);
  }
}
void render_system::add_queue_2d(const std::shared_ptr<drawable> draw_object) {
  m_drawable_2d.push_back(draw_object);
}
void render_system::add_queue_3d(const std::shared_ptr<drawable> draw_object) {
  m_drawable_3d.push_back(draw_object);
}
void render_system::add_queue_instancing(
    const std::shared_ptr<instancing> draw_object) {
  m_instancing.push_back(draw_object);
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
void render_system::remove_queue_instancing(
    const std::shared_ptr<instancing> draw_object) {
  m_instancing.erase(
      std::remove(m_instancing.begin(), m_instancing.end(), draw_object),
      m_instancing.end());
}
void render_system::add_vertex_array(const vertex_array &vArray,
                                     std::string_view name) {

#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_vertex_array(vArray, name);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->add_vertex_array(vArray, name);
  }
}

void render_system::update_vertex_array(const vertex_array &vArray,
                                        std::string_view name) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->update_vertex_array(vArray, name);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->update_vertex_array(vArray, name);
  }
}
void render_system::add_model(const model &m) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_model(m);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->add_model(m);
  }
}
void render_system::update_model(const model &m) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->update_model(m);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->update_model(m);
  }
}
void render_system::add_instancing(const instancing &_instancing) {
  if (_instancing.data.empty()) {
    return;
  }
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_instancing(_instancing);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->add_instancing(_instancing);
  }
}

void render_system::load_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->load_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->load_shader(shaderInfo);
  }
}

void render_system::unload_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->unload_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->unload_shader(shaderInfo);
  }
}
void *render_system::get_texture_id() {
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
  if (RendererAPI == graphics_api::Vulkan) {
    return m_vk_renderer->get_texture_id();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    return m_gl_renderer->get_texture_id();
  }
}

void render_system::setup_shapes() {
  add_vertex_array(create_box_vertices(), vertex_default_shapes::box);
  add_vertex_array(create_sprite_vertices(), vertex_default_shapes::sprite);
}
} // namespace sinen
