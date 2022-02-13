#include "Render/Renderer.hpp"
#include "../Vertex/default_model_creator.h"
#include "Math/Vector3.hpp"
#include "OpenGL/GLRenderer.h"
#include "Vertex/Vertex.hpp"
#include "Vertex/VertexArray.hpp"
#include "Vulkan/VKRenderer.h"
#include "src/Nen/Render/Vulkan/VKRenderer.h"
#include <Nen.hpp>

namespace nen {
renderer::renderer(graphics_api api, std::shared_ptr<window> window)
    : mWindow(window), m_renderer(nullptr), RendererAPI(api) {
  if (RendererAPI == graphics_api::Vulkan) {
    m_renderer = std::make_unique<vk::VKRenderer>();

  } else {
    m_renderer = std::make_unique<gl::GLRenderer>();
  }
  m_renderer->SetRenderer(this);
  m_renderer->Initialize(mWindow);
  setup_shapes();
}

void renderer::Shutdown() { m_renderer->Shutdown(); }

void renderer::UnloadData() {}

void renderer::Draw() { m_renderer->Render(); }

void renderer::AddDrawObject2D(std::shared_ptr<nen::draw_object> drawObject,
                               std::shared_ptr<texture> texture) {
  m_renderer->AddDrawObject2D(drawObject, texture);
}

void renderer::RemoveDrawObject2D(std::shared_ptr<draw_object> drawObject) {
  m_renderer->RemoveDrawObject2D(drawObject);
}

void renderer::AddDrawObject3D(std::shared_ptr<draw_object> drawObject,
                               std::shared_ptr<texture> texture) {
  m_renderer->AddDrawObject3D(drawObject, texture);
}

void renderer::RemoveDrawObject3D(std::shared_ptr<draw_object> drawObject) {
  m_renderer->RemoveDrawObject3D(drawObject);
}

void renderer::AddGUI(std::shared_ptr<ui_screen> ui) { m_renderer->AddGUI(ui); }

void renderer::RemoveGUI(std::shared_ptr<ui_screen> ui) {
  m_renderer->RemoveGUI(ui);
}

void renderer::AddVertexArray(const vertex_array &vArray,
                              std::string_view name) {
  m_renderer->AddVertexArray(vArray, name);
}

void renderer::UpdateVertexArray(const vertex_array &vArray,
                                 std::string_view name) {
  m_renderer->UpdateVertexArray(vArray, name);
}

void renderer::add_instancing(instancing &_instancing) {
  m_renderer->add_instancing(_instancing);
}
void renderer::remove_instancing(instancing &_instancing) {
  m_renderer->remove_instancing(_instancing);
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

} // namespace nen