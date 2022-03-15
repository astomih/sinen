#include "Render/Renderer.hpp"
#include "../Vertex/default_model_creator.h"
#include "Math/Vector3.hpp"
#include "OpenGL/GLRenderer.h"
#include "Vertex/Vertex.hpp"
#include "Vertex/VertexArray.hpp"
#include "Vulkan/VKRenderer.h"

namespace nen {
renderer::renderer(manager &_manager) : m_manager(_manager), showImGui(true) {}
void renderer::initialize(graphics_api api) {
  m_renderer = nullptr;
  RendererAPI = api;
  if (RendererAPI == graphics_api::Vulkan) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
    m_renderer = std::make_unique<vk::VKRenderer>(m_manager);
#endif
  } else {
    m_renderer = std::make_unique<gl::GLRenderer>(m_manager);
  }
  m_renderer->Initialize();
  setup_shapes();
}

void renderer::Shutdown() { m_renderer->Shutdown(); }

void renderer::UnloadData() {}

void renderer::render() { m_renderer->Render(); }
void renderer::draw2d(std::shared_ptr<nen::draw_object> drawObject) {
  m_renderer->draw2d(drawObject);
}

void renderer::draw3d(std::shared_ptr<draw_object> drawObject) {
  m_renderer->draw3d(drawObject);
}

void renderer::AddVertexArray(const vertex_array &vArray,
                              std::string_view name) {
  m_renderer->AddVertexArray(vArray, name);
}

void renderer::UpdateVertexArray(const vertex_array &vArray,
                                 std::string_view name) {
  m_renderer->UpdateVertexArray(vArray, name);
}

void renderer::add_instancing(const instancing &_instancing) {
  m_renderer->add_instancing(_instancing);
}

void renderer::LoadShader(const shader &shaderInfo) {
  m_renderer->LoadShader(shaderInfo);
}

void renderer::UnloadShader(const shader &shaderInfo) {
  m_renderer->UnloadShader(shaderInfo);
}

void renderer::setup_shapes() {
  AddVertexArray(create_box_vertices(), vertex_default_shapes::box);
  AddVertexArray(create_sprite_vertices(), vertex_default_shapes::sprite);
}
window &renderer::GetWindow() { return m_manager.get_window(); }

} // namespace nen