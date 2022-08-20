#include "../manager/get_system.hpp"
#include "../vertex/default_model_creator.h"
#include "opengl/gl_renderer.hpp"
#include "render_system.hpp"
#include "vulkan/vk_renderer.hpp"
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
render_system::render_system() : showImGui(false) {}
render_system::~render_system() = default;
renderer::~renderer() = default;
graphics_api render_system::get_graphics_api() { return this->RendererAPI; }
void render_system::unload_data() {}
void render_system::initialize(graphics_api api) {
  RendererAPI = api;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
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
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->shutdown();
  }
#endif
}

void render_system::render() {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->render();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->render();
  }
}
void render_system::draw2d(std::shared_ptr<drawable> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw2d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->draw2d(drawObject);
  }
}

void render_system::draw3d(std::shared_ptr<drawable> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw3d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->draw3d(drawObject);
  }
}

void render_system::add_vertex_array(const vertex_array &vArray,
                                     std::string_view name) {

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
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
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->update_vertex_array(vArray, name);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->update_vertex_array(vArray, name);
  }
}

void render_system::add_instancing(const instancing &_instancing) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_instancing(_instancing);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL || RendererAPI == graphics_api::ES) {
    m_gl_renderer->add_instancing(_instancing);
  }
}

void render_system::load_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->load_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->load_shader(shaderInfo);
  }
}

void render_system::unload_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->unload_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->unload_shader(shaderInfo);
  }
}

void render_system::setup_shapes() {
  add_vertex_array(create_box_vertices(), vertex_default_shapes::box);
  add_vertex_array(create_sprite_vertices(), vertex_default_shapes::sprite);
}
graphics_api renderer::get_graphics_api() {
  return get_renderer().get_graphics_api();
}

void renderer::unload_data() { get_renderer().unload_data(); }

void renderer::render() { get_renderer().render(); }

void renderer::draw2d(const std::shared_ptr<drawable> draw_object) {
  get_renderer().draw2d(draw_object);
}
void renderer::draw3d(const std::shared_ptr<drawable> draw_object) {
  get_renderer().draw3d(draw_object);
}

void renderer::add_vertex_array(const vertex_array &vArray,
                                std::string_view name) {
  get_renderer().add_vertex_array(vArray, name);
}
void renderer::update_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
  get_renderer().update_vertex_array(vArray, name);
}

void renderer::add_instancing(const instancing &_instancing) {
  get_renderer().add_instancing(_instancing);
}

void renderer::set_clear_color(const color &color) {
  get_renderer().set_clear_color(color);
}

color renderer::get_clear_color() { return get_renderer().get_clear_color(); }

void renderer::set_skybox_texture(texture _skybox_texture) {
  get_renderer().set_skybox_texture(_skybox_texture);
}

texture renderer::get_skybox_texture() {
  return get_renderer().get_skybox_texture();
}

void renderer::toggle_show_imgui() { get_renderer().toggle_show_imgui(); }
bool renderer::is_show_imgui() { return get_renderer().is_show_imgui(); }

void renderer::load_shader(const shader &shaderinfo) {
  get_renderer().load_shader(shaderinfo);
}
void renderer::unload_shader(const shader &shaderinfo) {
  get_renderer().unload_shader(shaderinfo);
}

std::list<std::function<void()>> &renderer::get_imgui_function() {
  return get_renderer().get_imgui_function();
}

void renderer::add_imgui_function(std::function<void()> function) {
  get_renderer().get_imgui_function().push_back(function);
}
} // namespace sinen
