#include "../asset/model/model_data.hpp"
#include "graphics_system.hpp"
#include <SDL3/SDL_events.h>
#include <asset/asset.hpp>
#include <cstring>
#include <graphics/camera/camera.hpp>
#include <graphics/drawable/drawable_wrapper.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/mat4x4.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
void Graphics::Draw2D(const sinen::Draw2D &draw2D) {
  GraphicsSystem::Draw2D(draw2D);
}
void Graphics::Draw3D(const sinen::Draw3D &draw3D) {
  GraphicsSystem::Draw3D(draw3D);
}
void Graphics::DrawRect(const Rect &rect, const Color &color, float angle) {
  GraphicsSystem::DrawRect(rect, color, angle);
}
void Graphics::DrawImage(const Texture &texture, const Rect &rect,
                         float angle) {
  GraphicsSystem::DrawImage(texture, rect, angle);
}

void Graphics::DrawText(const std::string &text, const glm::vec2 &position,
                        const Color &color, float fontSize, float angle) {
  GraphicsSystem::DrawText(text, position, color, fontSize, angle);
}
void Graphics::DrawModel(const Model &model, const Transform &transform,
                         const Material &material) {
  GraphicsSystem::DrawModel(model, transform, material);
}

void Graphics::SetClearColor(const Color &color) {
  GraphicsSystem::set_clear_color(color);
}
Color Graphics::GetClearColor() { return GraphicsSystem::get_clear_color(); }
void Graphics::toggle_show_imgui() { GraphicsSystem::toggle_show_imgui(); }
bool Graphics::is_show_imgui() { return GraphicsSystem::is_show_imgui(); }
std::list<std::function<void()>> &Graphics::get_imgui_function() {
  return GraphicsSystem::get_imgui_function();
}

void Graphics::add_imgui_function(std::function<void()> function) {
  GraphicsSystem::get_imgui_function().push_back(function);
}
void Graphics::SetCamera(const Camera &camera) {
  GraphicsSystem::SetCamera(camera);
}
Camera &Graphics::GetCamera() { return GraphicsSystem::GetCamera(); }

void Graphics::SetCamera2D(const Camera2D &camera) {
  GraphicsSystem::SetCamera2D(camera);
}

Camera2D &Graphics::GetCamera2D() { return GraphicsSystem::GetCamera2D(); }

void Graphics::BindPipeline3D(const GraphicsPipeline3D &pipeline) {
  GraphicsSystem::bind_pipeline3d(pipeline);
}
void Graphics::BindDefaultPipeline3D() {
  GraphicsSystem::bind_default_pipeline3d();
}
void Graphics::BindPipeline2D(const GraphicsPipeline2D &pipeline) {
  GraphicsSystem::bind_pipeline2d(pipeline);
}
void Graphics::BindDefaultPipeline2D() {
  GraphicsSystem::bind_default_pipeline2d();
}
void Graphics::SetUniformData(uint32_t slot, const UniformData &data) {
  GraphicsSystem::set_uniform_data(slot, data);
}
void Graphics::SetRenderTarget(const RenderTexture &texture) {
  GraphicsSystem::SetRenderTarget(texture);
}
void Graphics::Flush() { GraphicsSystem::Flush(); }
Texture Graphics::ReadbackTexture(const RenderTexture &texture) {
  return GraphicsSystem::ReadbackTexture(texture);
}
} // namespace sinen
