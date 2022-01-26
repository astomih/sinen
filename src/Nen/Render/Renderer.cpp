#include "Render/Renderer.hpp"
#include "Effect/Effect.hpp"
#include "OpenGL/GLRenderer.h"
#include "OpenGLES/ESRenderer.h"
#include "RendererHandle.hpp"
#include "Vertex/Vertex.hpp"
#include "Vertex/VertexArray.hpp"
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
  m_renderer = std::make_unique<es::ESRenderer>();
#endif
  m_renderer->SetRenderer(this);
  m_renderer->Initialize(mWindow);
  setup_shapes();
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
void create_box_vertices(renderer *_renderer) {
  const float value = 1.f;
  const vector2 lb(0.0f, 0.0f);
  const vector2 lt(0.f, 1.f);
  const vector2 rb(1.0f, 0.0f);
  const vector2 rt(1.0f, 1.0f);
  vector3 norm{1, 1, 1};
  const vector3 red{1.0f, 0.0f, 0.0f};
  const vector3 green{0.0f, 1.0f, 0.0f};
  const vector3 blue{0.0f, 0.0f, 1.0f};
  const vector3 white{1.0f, 1, 1};
  const vector3 black{0.0f, 0, 0};
  const vector3 yellow{1.0f, 1.0f, 0.0f};
  const vector3 magenta{1.0f, 0.0f, 1.0f};
  const vector3 cyan{0.0f, 1.0f, 1.0f};

  vertex_array vArray;
  vArray.vertices.push_back({vector3(-value, value, value), yellow, lb});
  vArray.vertices.push_back({vector3(-value, -value, value), red, lt});
  vArray.vertices.push_back({vector3(value, value, value), white, rb});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, rt});

  vArray.vertices.push_back({vector3(value, value, value), white, lb});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, lt});
  vArray.vertices.push_back({vector3(value, value, -value), cyan, rb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, rt});

  vArray.vertices.push_back({vector3(-value, value, -value), green, lb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, lt});
  vArray.vertices.push_back({vector3(-value, value, value), yellow, rb});
  vArray.vertices.push_back({vector3(-value, -value, value), red, rt});

  vArray.vertices.push_back({vector3(value, value, -value), cyan, lb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, lt});
  vArray.vertices.push_back({vector3(-value, value, -value), green, rb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, rt});

  vArray.vertices.push_back({vector3(-value, value, -value), green, lb});
  vArray.vertices.push_back({vector3(-value, value, value), yellow, lt});
  vArray.vertices.push_back({vector3(value, value, -value), cyan, rb});
  vArray.vertices.push_back({vector3(value, value, value), white, rt});

  vArray.vertices.push_back({vector3(-value, -value, value), red, lb});
  vArray.vertices.push_back({vector3(-value, -value, -value), black, lt});
  vArray.vertices.push_back({vector3(value, -value, value), magenta, rb});
  vArray.vertices.push_back({vector3(value, -value, -value), blue, rt});

  uint32_t indices[] = {
      0,  2,  1,  1,  2,  3,  // front
      4,  6,  5,  5,  6,  7,  // right
      8,  10, 9,  9,  10, 11, // left

      12, 14, 13, 13, 14, 15, // back
      16, 18, 17, 17, 18, 19, // top
      20, 22, 21, 21, 22, 23, // bottom
  };
  vArray.indexCount = sizeof(indices) / sizeof(uint32_t);
  vArray.push_indices(indices, vArray.indexCount);
  auto vArraySize = vArray.vertices.size() * sizeof(vertex);
  _renderer->AddVertexArray(vArray, "BOX");
}

void create_sprite_vertices(renderer *_renderer) {
  const float value = 1.f;
  const vector2 lb(0.0f, 0.0f);
  const vector2 lt(0.f, 1.f);
  const vector2 rb(1.0f, 0.0f);
  const vector2 rt(1.0f, 1.0f);
  vector3 norm{1, 1, 1};

  vertex_array vArray;
  vArray.vertices.push_back({vector3(-value, value, 0.5f), norm, lb});
  vArray.vertices.push_back({vector3(-value, -value, 0.5f), norm, lt});
  vArray.vertices.push_back({vector3(value, value, 0.5f), norm, rb});
  vArray.vertices.push_back({vector3(value, -value, 0.5f), norm, rt});

  uint32_t indices[] = {
      0, 2, 1, 1, 2, 3, // front
  };
  vArray.indexCount = sizeof(indices) / sizeof(uint32_t);
  vArray.push_indices(indices, vArray.indexCount);
  _renderer->AddVertexArray(vArray, "SPRITE");
}
void renderer::setup_shapes() {
  create_box_vertices(this);
  create_sprite_vertices(this);
}

} // namespace nen