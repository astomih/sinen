#include "../asset/model/model_data.hpp"
#include "graphics_system.hpp"
#include <SDL3/SDL_events.h>
#include <asset/asset.hpp>
#include <cstring>
#include <graphics/camera/camera.hpp>
#include <graphics/drawable/drawable.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

#include <graphics/graphics.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>

namespace sinen {
void Graphics::draw2D(const sinen::Draw2D &draw2D) {
  GraphicsSystem::drawBase2D(draw2D);
}
void Graphics::draw3D(const sinen::Draw3D &draw3D) {
  GraphicsSystem::drawBase3D(draw3D);
}
void Graphics::drawRect(const Rect &rect, const Color &color, float angle) {
  GraphicsSystem::drawRect(rect, color, angle);
}
void Graphics::drawImage(const Texture &texture, const Rect &rect,
                         float angle) {
  GraphicsSystem::drawImage(texture, rect, angle);
}

void Graphics::drawText(StringView text, const Font &font, const Vec2 &position,
                        const Color &color, float textSize, float angle) {
  GraphicsSystem::drawText(text, font, position, color, textSize, angle);
}
void Graphics::drawCubemap(const Cubemap &cubemap) {
  GraphicsSystem::drawCubemap(cubemap);
}
void Graphics::drawModel(const Model &model, const Transform &transform,
                         const Material &material) {
  GraphicsSystem::drawModel(model, transform, material);
}
void Graphics::drawModelInstanced(const Model &model,
                                  const Array<Transform> &transforms,
                                  const Material &material) {
  GraphicsSystem::drawModelInstanced(model, transforms, material);
}

void Graphics::setClearColor(const Color &color) {
  GraphicsSystem::setClearColor(color);
}
Color Graphics::getClearColor() { return GraphicsSystem::getClearColor(); }
void Graphics::toggleShowImGui() { GraphicsSystem::toggleShowImGui(); }
bool Graphics::isShowImGui() { return GraphicsSystem::isShowImGui(); }
std::list<std::function<void()>> &Graphics::getImGuiFunction() {
  return GraphicsSystem::getImGuiFunction();
}

void Graphics::addImGuiFunction(std::function<void()> function) {
  GraphicsSystem::getImGuiFunction().push_back(function);
}
void Graphics::setCamera(const Camera &camera) {
  GraphicsSystem::setCamera(camera);
}
Camera &Graphics::getCamera() { return GraphicsSystem::getCamera(); }

void Graphics::setCamera2D(const Camera2D &camera) {
  GraphicsSystem::setCamera2D(camera);
}

Camera2D &Graphics::getCamera2D() { return GraphicsSystem::getCamera2D(); }

void Graphics::bindPipeline(const GraphicsPipeline &pipeline) {
  GraphicsSystem::bindPipeline(pipeline);
}
void Graphics::setUniformBuffer(uint32_t slot, const Buffer &data) {
  GraphicsSystem::setUniformBuffer(slot, data);
}
void Graphics::setRenderTarget(const RenderTexture &texture) {
  GraphicsSystem::setRenderTarget(texture);
}
void Graphics::flush() { GraphicsSystem::flush(); }
bool Graphics::readbackTexture(const RenderTexture &texture, Texture &out) {
  return GraphicsSystem::readbackTexture(texture, out);
}
} // namespace sinen
