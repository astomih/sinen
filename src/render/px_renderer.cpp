#include "px_renderer.hpp"

namespace sinen {
void PxRenderer::initialize() {
  allocator = px::Paranoixa ::CreateAllocator(65536);
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
}
void PxRenderer::shutdown() {}
void PxRenderer::unload_data() {}
void PxRenderer::render() {}
void PxRenderer::draw2d(const std::shared_ptr<Drawable> draw_object) {}
void PxRenderer::drawui(const std::shared_ptr<Drawable> draw_object) {}
void PxRenderer::draw3d(const std::shared_ptr<Drawable> draw_object) {}
void PxRenderer::add_vertex_array(const VertexArray &vArray,
                                  std::string_view name) {}
void PxRenderer::update_vertex_array(const VertexArray &vArray,
                                     std::string_view name) {}
void PxRenderer::add_model(const Model &m) {}
void PxRenderer::update_model(const Model &m) {}
void PxRenderer::load_shader(const Shader &shaderinfo) {}
void PxRenderer::unload_shader(const Shader &shaderinfo) {}
void PxRenderer::prepare_imgui() {}
void *PxRenderer::get_texture_id() { return nullptr; }
} // namespace sinen