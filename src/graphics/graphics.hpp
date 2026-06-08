#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
// std
#include <functional>
#include <list>
#include <optional>
// internal
#include "graphics_pipeline.hpp"
#include <core/allocator/pool_allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <gpu/gpu_backend_api.hpp>
#include <graphics/camera/camera2d.hpp>
#include <graphics/camera/camera3d.hpp>
#include <graphics/font/font.hpp>
#include <graphics/model/model.hpp>
#include <graphics/text_style.hpp>
#include <graphics/texture/render_texture.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/geometry/rect.hpp>
#include <math/math.hpp>
#include <math/transform/transform.hpp>

namespace sinen {
namespace gpu {
class AccelerationStructure;
}

class Graphics {
public:
  static bool initialize();
  static void shutdown();
  static GPUBackendAPI getBackendAPI();
  static String getBackendName();
  static String getBackendName(GPUBackendAPI api);
  static bool parseBackendName(StringView name, GPUBackendAPI &api);
  static void render();
  static void begin2D();
  static void begin2D(const Camera2D &camera);
  static void begin3D(const Camera3D &camera);
  static void finish();
  static Vec2 windowToCurrent2D(const Vec2 &windowPosition);
  static void drawRect(const Rect &rect, const Color &color, float angle);
  static void drawRect(const Rect &rect, const Color &color) {
    drawRect(rect, color, 0.0f);
  }
  static void drawImage(const Ptr<Texture> &texture, const Rect &rect,
                        float angle);
  static void drawImage(const Ptr<Texture> &texture, const Rect &rect) {
    drawImage(texture, rect, 0.0f);
  }
  static void drawText(StringView text, const TextStyle &style,
                       const TextTransform &transform);
  static void drawCubemap(const Ptr<Texture> &cubemap);
  static void drawModel(const Model &model, const Transform &transform);
  static void drawModelInstanced(const Model &model,
                                 const Array<Transform> &transforms);
  static void setClearColor(const Color &color);
  static Color getClearColor();
  static void addPreDrawFunc(std::function<void()> f);
  static void addPostDrawFunc(std::function<void()> function);
  static void setGraphicsPipeline(const GraphicsPipeline &pipeline);
  static void resetGraphicsPipeline();
  static void setUniformBuffer(UInt32 slotIndex, const Buffer &buffer);
  static void setTexture(UInt32 slotIndex, const Ptr<Texture> &texture);
  static void resetTexture(UInt32 slotIndex);
  static void resetAllTexture();
  static void setAccelerationStructure(
      UInt32 slotIndex,
      const Ptr<gpu::AccelerationStructure> &accelerationStructure);
  static void resetAccelerationStructure(UInt32 slotIndex);
  static void resetAllAccelerationStructures();
  static void beginRenderTarget(const RenderTexture &texture);
  static void endRenderTarget();
  static bool readbackTexture(const RenderTexture &texture, Ptr<Texture> &out);

  static Ptr<gpu::Device> getDevice();
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
