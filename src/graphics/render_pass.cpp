#include <core/logger/log.hpp>
#include <graphics/graphics.hpp>
#include <graphics/render_pass.hpp>

namespace sinen {
RenderPass::RenderPass(Type type, UInt64 frameId,
                       std::optional<Camera2D> camera2D,
                       std::optional<Camera3D> camera3D,
                       std::optional<RenderTexture> target)
    : type(type), frameId(frameId), camera2D(std::move(camera2D)),
      camera3D(std::move(camera3D)), target(std::move(target)) {}

RenderPass::~RenderPass() = default;

bool RenderPass::activate() { return Graphics::activatePass(*this); }
bool RenderPass::activate(const Material &material) {
  return Graphics::activatePass(*this, material);
}

void RenderPass::setGraphicsPipeline(const GraphicsPipeline &pipeline) {
  if (activate()) {
    graphicsPipeline = pipeline;
  }
}

void RenderPass::resetGraphicsPipeline() {
  if (activate()) {
    graphicsPipeline = std::nullopt;
  }
}

void RenderPass::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  if (activate()) {
    Graphics::setUniformBuffer(slotIndex, buffer);
  }
}

void RenderPass::setUniformBuffer(StringView name, const Buffer &buffer) {
  if (activate()) {
    Graphics::setUniformBuffer(name, buffer);
  }
}

void RenderPass::setTexture(UInt32 slotIndex, const Ptr<Texture> &texture) {
  if (activate()) {
    textureBindings.insert_or_assign(slotIndex, texture);
  }
}

void RenderPass::setTexture(StringView name, const Ptr<Texture> &texture) {
  if (!activate()) {
    return;
  }
  if (!graphicsPipeline.has_value() || !graphicsPipeline->get()) {
    Log::error("RenderPass::setTexture('%.*s', ...) requires a ready custom "
               "graphics pipeline",
               static_cast<int>(name.size()), name.data());
    return;
  }
  UInt32 slot = 0;
  if (graphicsPipeline->findTextureSlot(name, slot)) {
    textureBindings.insert_or_assign(slot, texture);
  } else {
    Log::error("Texture '%.*s' was not found in the render pass pipeline",
               static_cast<int>(name.size()), name.data());
  }
}

void RenderPass::resetTexture(UInt32 slotIndex) {
  if (activate()) {
    textureBindings.erase(slotIndex);
  }
}

void RenderPass::resetTexture(StringView name) {
  if (!activate()) {
    return;
  }
  if (!graphicsPipeline.has_value() || !graphicsPipeline->get()) {
    Log::error("RenderPass::resetTexture(name) requires a ready custom "
               "graphics pipeline");
    return;
  }
  UInt32 slot = 0;
  if (graphicsPipeline->findTextureSlot(name, slot)) {
    textureBindings.erase(slot);
  } else {
    Log::error("Texture '%.*s' was not found in the render pass pipeline",
               static_cast<int>(name.size()), name.data());
  }
}

void RenderPass::resetAllTexture() {
  if (activate()) {
    textureBindings.clear();
  }
}

void RenderPass::setAccelerationStructure(
    UInt32 slotIndex,
    const Ptr<gpu::AccelerationStructure> &accelerationStructure) {
  if (activate()) {
    accelerationStructureBindings.insert_or_assign(slotIndex,
                                                   accelerationStructure);
  }
}

void RenderPass::resetAccelerationStructure(UInt32 slotIndex) {
  if (activate()) {
    accelerationStructureBindings.erase(slotIndex);
  }
}

void RenderPass::resetAllAccelerationStructures() {
  if (activate()) {
    accelerationStructureBindings.clear();
  }
}

Render2DPass::Render2DPass(UInt64 frameId, std::optional<Camera2D> camera,
                           std::optional<RenderTexture> target)
    : RenderPass(Type::TwoD, frameId, std::move(camera), std::nullopt,
                 std::move(target)) {}

Vec2 Render2DPass::windowToCurrent(const Vec2 &windowPosition) {
  return activate() ? Graphics::windowToCurrent2D(windowPosition)
                    : windowPosition;
}

void Render2DPass::drawRect(const Rect &rect, const Color &color, float angle) {
  if (activate()) {
    Graphics::drawRect(rect, color, angle);
  }
}

void Render2DPass::drawRect(const Rect &rect, const Color &color,
                            const Material &material, float angle) {
  if (activate(material)) {
    Graphics::drawRect(rect, color, angle);
  }
}

void Render2DPass::drawRects(const Array<Rect> &rects, const Color &color) {
  if (activate()) {
    Graphics::drawRects(rects, color);
  }
}

void Render2DPass::drawImage(const Ptr<Texture> &texture, const Rect &rect,
                             float angle) {
  if (activate()) {
    Graphics::drawImage(texture, rect, angle);
  }
}

void Render2DPass::drawImage(const Ptr<Texture> &texture, const Rect &rect,
                             const Material &material, float angle) {
  if (activate(material)) {
    Graphics::drawImage(texture, rect, angle);
  }
}

void Render2DPass::drawText(StringView text, const TextStyle &style,
                            const TextTransform &transform) {
  if (activate()) {
    Graphics::drawText(text, style, transform);
  }
}

void Render2DPass::drawText(StringView text, const TextStyle &style,
                            const TextTransform &transform,
                            const Material &material) {
  if (activate(material)) {
    Graphics::drawText(text, style, transform);
  }
}

void Render2DPass::drawTexts(const Array<TextBatchItem> &items,
                             const TextStyle &style) {
  if (activate()) {
    Graphics::drawTexts(items, style);
  }
}

Render3DPass::Render3DPass(UInt64 frameId, const Camera3D &camera,
                           std::optional<RenderTexture> target)
    : RenderPass(Type::ThreeD, frameId, std::nullopt, camera,
                 std::move(target)) {}

void Render3DPass::drawCubemap(const Ptr<Texture> &cubemap) {
  if (activate()) {
    Graphics::drawCubemap(cubemap);
  }
}

void Render3DPass::drawCubemap(const Ptr<Texture> &cubemap,
                               const Material &material) {
  if (activate(material)) {
    Graphics::drawCubemap(cubemap);
  }
}

void Render3DPass::drawModel(const Model &model, const Transform &transform) {
  if (activate()) {
    Graphics::drawModel(model, transform);
  }
}

void Render3DPass::drawModel(const Model &model, const Transform &transform,
                             const Material &material) {
  if (activate(material)) {
    Graphics::drawModel(model, transform);
  }
}

void Render3DPass::drawModelInstanced(const Model &model,
                                      const Array<Transform> &transforms) {
  if (activate()) {
    Graphics::drawModelInstanced(model, transforms);
  }
}

void Render3DPass::drawModelInstanced(const Model &model,
                                      const Array<Transform> &transforms,
                                      const Material &material) {
  if (activate(material)) {
    Graphics::drawModelInstanced(model, transforms);
  }
}
} // namespace sinen
