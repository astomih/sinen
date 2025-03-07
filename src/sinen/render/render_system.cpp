#include "render_system.hpp"
#include "../model/default_model_creator.hpp"
#include "px_renderer.hpp"
#include <cstdint>
#include <io/asset_type.hpp>
#include <io/data_stream.hpp>
#include <math/vector3.hpp>
#include <model/vertex.hpp>
#include <model/vertex_array.hpp>
#include <render/renderer.hpp>

#include <imgui.h>

namespace sinen {
Color RendererImpl::clearColor = Palette::black();
std::shared_ptr<PxRenderer> RendererImpl::pxRenderer =
    std::make_shared<PxRenderer>(px::Paranoixa::CreateAllocator(0xffff));
// Renderer
bool RendererImpl::showImGui = false;
std::list<std::function<void()>> RendererImpl::m_imgui_function;
Model RendererImpl::box = Model();
Model RendererImpl::sprite = Model();
void RendererImpl::unload_data() {}
void RendererImpl::initialize() {
  pxRenderer->initialize();
  setup_shapes();
}

void RendererImpl::shutdown() {
  pxRenderer->shutdown();
  pxRenderer.reset();
}

void RendererImpl::render() { pxRenderer->render(); }
void RendererImpl::draw2d(std::shared_ptr<Drawable> drawObject) {
  pxRenderer->draw2d(drawObject);
}

void RendererImpl::draw3d(std::shared_ptr<Drawable> drawObject) {
  pxRenderer->draw3d(drawObject);
}
void RendererImpl::load_shader(const Shader &shaderInfo) {
  pxRenderer->load_shader(shaderInfo);
}
void RendererImpl::unload_shader(const Shader &shaderInfo) {
  pxRenderer->unload_shader(shaderInfo);
}
void *RendererImpl::get_texture_id() { return pxRenderer->get_texture_id(); }

void RendererImpl::setup_shapes() {
  box.load_from_vertex_array(create_box_vertices());
  sprite.load_from_vertex_array(create_sprite_vertices());
}

void RendererImpl::prepare_imgui() {
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontFromFileTTF(DataStream::convert_file_path(
                                   AssetType::Font, "mplus/mplus-1p-medium.ttf")
                                   .data(),
                               18.0f, nullptr,
                               io.Fonts->GetGlyphRangesJapanese());
}
void RendererImpl::begin_pipeline3d(const RenderPipeline3D &pipeline) {
  pxRenderer->begin_pipeline3d(pipeline);
}
void RendererImpl::end_pipeline3d() { pxRenderer->end_pipeline3d(); }
void RendererImpl::begin_pipeline2d(const RenderPipeline2D &pipeline) {
  pxRenderer->begin_pipeline2d(pipeline);
}
void RendererImpl::end_pipeline2d() { pxRenderer->end_pipeline2d(); }
void RendererImpl::set_uniform_data(uint32_t slot, const UniformData &data) {
  pxRenderer->set_uniform_data(slot, data);
}
} // namespace sinen
