#include "render_system.hpp"
#include "../asset/model/default_model_creator.hpp"
#include "px_renderer.hpp"
#include <asset/model/vertex.hpp>
#include <asset/model/vertex_array.hpp>
#include <asset/texture/render_texture.hpp>
#include <core/io/asset_type.hpp>
#include <core/io/data_stream.hpp>
#include <cstdint>
#include <render/renderer.hpp>

#include <imgui.h>

#include "../asset/font/default/mplus-1p-medium.ttf.h"

namespace sinen {
Color RendererSystem::clearColor = Palette::black();
std::shared_ptr<PxRenderer> RendererSystem::pxRenderer =
    std::make_shared<PxRenderer>(px::Paranoixa::CreateAllocator(0xffff));
// Renderer
bool RendererSystem::showImGui = false;
std::list<std::function<void()>> RendererSystem::m_imgui_function;
Model RendererSystem::box = Model();
Model RendererSystem::sprite = Model();
void RendererSystem::unload_data() {}
void RendererSystem::initialize() {
  pxRenderer->initialize();
  setup_shapes();
}

void RendererSystem::shutdown() {
  pxRenderer->shutdown();
  pxRenderer.reset();
}

void RendererSystem::render() { pxRenderer->render(); }
void RendererSystem::draw2d(std::shared_ptr<Drawable> drawObject) {
  pxRenderer->draw2d(drawObject);
}

void RendererSystem::draw3d(std::shared_ptr<Drawable> drawObject) {
  pxRenderer->draw3d(drawObject);
}
void RendererSystem::load_shader(const Shader &shaderInfo) {
  pxRenderer->load_shader(shaderInfo);
}
void RendererSystem::unload_shader(const Shader &shaderInfo) {
  pxRenderer->unload_shader(shaderInfo);
}
void *RendererSystem::get_texture_id() { return pxRenderer->get_texture_id(); }

void RendererSystem::setup_shapes() {
  box.load_from_vertex_array(create_box_vertices());
  sprite.load_from_vertex_array(create_sprite_vertices());
}

void RendererSystem::prepare_imgui() {
  ImGuiIO &io = ImGui::GetIO();
  io.WantTextInput = true;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontFromMemoryTTF((void *)mplus_1p_medium_ttf,
                                 mplus_1p_medium_ttf_len, 18.0f, nullptr,
                                 io.Fonts->GetGlyphRangesJapanese());
}
void RendererSystem::begin_pipeline3d(const RenderPipeline3D &pipeline) {
  pxRenderer->begin_pipeline3d(pipeline);
}
void RendererSystem::end_pipeline3d() { pxRenderer->end_pipeline3d(); }
void RendererSystem::begin_pipeline2d(const RenderPipeline2D &pipeline) {
  pxRenderer->begin_pipeline2d(pipeline);
}
void RendererSystem::end_pipeline2d() { pxRenderer->end_pipeline2d(); }
void RendererSystem::set_uniform_data(uint32_t slot, const UniformData &data) {
  pxRenderer->set_uniform_data(slot, data);
}
void RendererSystem::begin_render_texture2d(const RenderTexture &texture) {
  pxRenderer->begin_render_texture2d(texture);
}
void RendererSystem::begin_render_texture3d(const RenderTexture &texture) {
  pxRenderer->begin_render_texture3d(texture);
}
void RendererSystem::end_render_texture(const RenderTexture &texture,
                                      Texture &out) {
  pxRenderer->end_render_texture(texture, out);
}
} // namespace sinen
