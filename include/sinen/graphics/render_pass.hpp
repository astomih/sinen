#ifndef SINEN_GRAPHICS_RENDER_PASS_HPP
#define SINEN_GRAPHICS_RENDER_PASS_HPP

#include <core/buffer/buffer.hpp>
#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <graphics/camera/camera2d.hpp>
#include <graphics/camera/camera3d.hpp>
#include <graphics/graphics_pipeline.hpp>
#include <graphics/material.hpp>
#include <graphics/model/model.hpp>
#include <graphics/text_style.hpp>
#include <graphics/texture/render_texture.hpp>
#include <graphics/texture/texture.hpp>
#include <math/color/color.hpp>
#include <math/geometry/rect.hpp>
#include <math/transform/transform.hpp>

#include <optional>

namespace sinen {
namespace gpu {
class AccelerationStructure;
}

class Graphics;

// A frame-scoped, high-level rendering context. Graphics owns the underlying
// GPU command buffer and closes the pass through Graphics::endPass().
class RenderPass {
public:
  enum class Type { TwoD, ThreeD };

  virtual ~RenderPass();

  bool isFinished() const { return finished; }

  void setGraphicsPipeline(const GraphicsPipeline &pipeline);
  void resetGraphicsPipeline();
  void setUniformBuffer(UInt32 slotIndex, const Buffer &buffer);
  void setUniformBuffer(StringView name, const Buffer &buffer);
  void setTexture(UInt32 slotIndex, const Ptr<Texture> &texture);
  void setTexture(StringView name, const Ptr<Texture> &texture);
  void resetTexture(UInt32 slotIndex);
  void resetTexture(StringView name);
  void resetAllTexture();
  void setAccelerationStructure(
      UInt32 slotIndex,
      const Ptr<gpu::AccelerationStructure> &accelerationStructure);
  void resetAccelerationStructure(UInt32 slotIndex);
  void resetAllAccelerationStructures();

protected:
  RenderPass(Type type, UInt64 frameId, std::optional<Camera2D> camera2D,
             std::optional<Camera3D> camera3D,
             std::optional<RenderTexture> target);
  bool activate();
  bool activate(const Material &material);

private:
  friend class Graphics;

  Type type;
  UInt64 frameId;
  bool finished = false;
  std::optional<Camera2D> camera2D;
  std::optional<Camera3D> camera3D;
  std::optional<RenderTexture> target;
  std::optional<GraphicsPipeline> graphicsPipeline;
  Hashmap<UInt32, Ptr<Texture>> textureBindings;
  Hashmap<UInt32, Ptr<gpu::AccelerationStructure>>
      accelerationStructureBindings;
};

class Render2DPass final : public RenderPass {
public:
  static constexpr const char *metaTableName() { return "sn.Render2DPass"; }

  Render2DPass(UInt64 frameId, std::optional<Camera2D> camera,
               std::optional<RenderTexture> target);

  Vec2 windowToCurrent(const Vec2 &windowPosition);
  void drawRect(const Rect &rect, const Color &color, float angle = 0.0f);
  void drawRect(const Rect &rect, const Color &color, const Material &material,
                float angle = 0.0f);
  void drawRects(const Array<Rect> &rects, const Color &color);
  void drawImage(const Ptr<Texture> &texture, const Rect &rect,
                 float angle = 0.0f);
  void drawImage(const Ptr<Texture> &texture, const Rect &rect,
                 const Material &material, float angle = 0.0f);
  void drawText(StringView text, const TextStyle &style,
                const TextTransform &transform);
  void drawText(StringView text, const TextStyle &style,
                const TextTransform &transform, const Material &material);
  void drawTexts(const Array<TextBatchItem> &items, const TextStyle &style);
};

class Render3DPass final : public RenderPass {
public:
  static constexpr const char *metaTableName() { return "sn.Render3DPass"; }

  Render3DPass(UInt64 frameId, const Camera3D &camera,
               std::optional<RenderTexture> target);

  void drawCubemap(const Ptr<Texture> &cubemap);
  void drawCubemap(const Ptr<Texture> &cubemap, const Material &material);
  void drawModel(const Model &model, const Transform &transform);
  void drawModel(const Model &model, const Transform &transform,
                 const Material &material);
  void drawModelInstanced(const Model &model,
                          const Array<Transform> &transforms);
  void drawModelInstanced(const Model &model,
                          const Array<Transform> &transforms,
                          const Material &material);
};
} // namespace sinen

#endif // SINEN_GRAPHICS_RENDER_PASS_HPP
