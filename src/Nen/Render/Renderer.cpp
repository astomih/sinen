#include "Effect/Effect.hpp"
#include "OpenGL/GLRenderer.h"
#include "OpenGLES/ESRenderer.h"
#include "RendererHandle.hpp"
#include "Vulkan/VKRenderer.h"
#include <Nen.hpp>

namespace nen {
std::shared_ptr<renderer> renderer_handle::mRenderer = nullptr;
renderer::renderer(graphics_api api, std::shared_ptr<window> window)
    : transPic(nullptr), mScene(nullptr), mWindow(window), m_renderer(nullptr),
      RendererAPI(api) {
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  switch (RendererAPI) {
  case graphics_api::Vulkan:
    m_renderer = std::make_unique<vk::VKRenderer>();
    break;
  case graphics_api::OpenGL:
    m_renderer = std::make_unique<gl::GLRenderer>();
    break;
  default:
    break;
  }
#endif
#if defined(EMSCRIPTEN) || defined(MOBILE)
  renderer = std::make_unique<es::ESRenderer>();
#endif
  m_renderer->SetRenderer(this);
  m_renderer->Initialize(mWindow);
}

bool renderer::Initialize(std::shared_ptr<base_scene> scene,
                          std::shared_ptr<Transition> transition) {
  return true;
}

void renderer::Shutdown() { m_renderer->Shutdown(); }

void renderer::UnloadData() {}

void renderer::Draw() {
  for (auto e = mEffects.begin(); e != mEffects.end();) {
    if ((*e)->state == effect::State::Dead)
      e = mEffects.erase(e);
    if (e != mEffects.end())
      e++;
  }
  m_renderer->Render();
}

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

void renderer::AddEffect(std::shared_ptr<effect> effect) {
  m_renderer->LoadEffect(effect);
  mEffects.emplace_back(effect);
}
void renderer::RemoveEffect(std::shared_ptr<effect> effect) {
  auto iter = std::find(mEffects.begin(), mEffects.end(), effect);
  mEffects.erase(iter);
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

void renderer::LoadShader(const shader &shaderInfo) {
  m_renderer->LoadShader(shaderInfo);
}

void renderer::UnloadShader(const shader &shaderInfo) {
  m_renderer->UnloadShader(shaderInfo);
}

} // namespace nen
