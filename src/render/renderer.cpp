#include "../vertex/default_model_creator.h"
#include "opengl/gl_renderer.hpp"
#include "vulkan/vk_renderer.hpp"
#include <math/vector3.hpp>
#include <render/renderer.hpp>
#include <vertex/vertex.hpp>
#include <vertex/vertex_array.hpp>

namespace nen {
renderer::renderer(manager &_manager) : m_manager(_manager), showImGui(false) {}
renderer::~renderer() = default;
void renderer::initialize(graphics_api api) {
  RendererAPI = api;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer = std::make_unique<vk_renderer>();
    m_vk_renderer->initialize();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer = std::make_unique<gl_renderer>();
    m_gl_renderer->initialize();
  }
  setup_shapes();
}

void renderer::shutdown() {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->shutdown();
  }
#endif
}

void renderer::unload_data() {}

void renderer::render() {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->render();
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->render();
  }
}
void renderer::draw2d(std::shared_ptr<nen::draw_object> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw2d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->draw2d(drawObject);
  }
}

void renderer::draw3d(std::shared_ptr<draw_object> drawObject) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->draw3d(drawObject);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->draw3d(drawObject);
  }
}

void renderer::add_vertex_array(const vertex_array &vArray,
                                std::string_view name) {

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_vertex_array(vArray, name);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->add_vertex_array(vArray, name);
  }
}

void renderer::update_vertex_array(const vertex_array &vArray,
                                   std::string_view name) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->update_vertex_array(vArray, name);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->update_vertex_array(vArray, name);
  }
}

void renderer::add_instancing(const instancing &_instancing) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->add_instancing(_instancing);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->add_instancing(_instancing);
  }
}

void renderer::load_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->load_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->load_shader(shaderInfo);
  }
}

void renderer::unload_shader(const shader &shaderInfo) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  if (RendererAPI == graphics_api::Vulkan) {
    m_vk_renderer->unload_shader(shaderInfo);
  }
#endif
  if (RendererAPI == graphics_api::OpenGL) {
    m_gl_renderer->unload_shader(shaderInfo);
  }
}

void renderer::setup_shapes() {
  add_vertex_array(create_box_vertices(), vertex_default_shapes::box);
  add_vertex_array(create_sprite_vertices(), vertex_default_shapes::sprite);
}
} // namespace nen