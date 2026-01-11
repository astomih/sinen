#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
// std
#include <functional>
#include <list>
// internal
#include "graphics_pipeline.hpp"
#include <asset/asset.hpp>
#include <core/allocator/pool_allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <graphics/camera/camera.hpp>
#include <graphics/camera/camera2d.hpp>
#include <graphics/drawable/drawable.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/geometry/rect.hpp>
#include <math/math.hpp>

namespace sinen {
class Graphics {
public:
  static bool initialize();
  static void shutdown();
  static void render();
  static void setCamera2D(const Camera2D &camera) {
    Graphics::camera2D = camera;
  }
  static Camera2D &getCamera2D() { return camera2D; }
  static void setCamera(const Camera &camera) { Graphics::camera = camera; }
  static Camera &getCamera() { return camera; }
  static void drawBase2D(const sinen::Draw2D &draw2D);
  static void drawBase3D(const sinen::Draw3D &draw3D);
  static void drawRect(const Rect &rect, const Color &color, float angle);
  static void drawRect(const Rect &rect, const Color &color) {
    drawRect(rect, color, 0.0f);
  }
  static void drawImage(const Texture &texture, const Rect &rect, float angle);
  static void drawImage(const Texture &texture, const Rect &rect) {
    drawImage(texture, rect, 0.0f);
  }
  static void drawText(StringView text, const Font &font, const Vec2 &position,
                       const Color &color, float textSize, float angle);
  static void drawText(StringView text, const Font &font, const Vec2 &position,
                       const Color &color) {
    drawText(text, font, position, color, 32.f, 0.0f);
  }
  static void drawText(StringView text, const Font &font,
                       const Vec2 &position) {
    drawText(text, font, position, Palette::white(), 32.f, 0.0f);
  }
  static void drawCubemap(const Texture &cubemap);
  static void drawModel(const Model &model, const Transform &transform,
                        const Material &material);
  static void drawModelInstanced(const Model &model,
                                 const Array<Transform> &transforms,
                                 const Material &material);
  static void setClearColor(const Color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }
  static Color getClearColor() { return clearColor; }
  static void toggleShowImGui() { showImGui = !showImGui; }
  static bool isShowImGui() { return showImGui; }
  static std::list<std::function<void()>> &getImGuiFunction() {
    return imguiFunctions;
  }
  static void addPreDrawFunc(std::function<void()> f) {
    preDrawFuncs.push_back(f);
  }
  static void addImGuiFunction(std::function<void()> function) {
    imguiFunctions.push_back(function);
  }
  static void setRenderTarget(const RenderTexture &texture);
  static void flush();
  static bool readbackTexture(const RenderTexture &texture, Texture &out);

  static Model box;
  static Model sprite;

  static Ptr<rhi::Device> getDevice() { return device; }

private:
  static void beginRenderPass(bool depthEnabled, rhi::LoadOp loadOp);
  static void prepareRenderPassFrame();
  static void setupShapes();
  static Color clearColor;

  inline static Camera camera;
  inline static Camera2D camera2D;
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> imguiFunctions;
  static std::list<std::function<void()>> preDrawFuncs;

  inline static Ptr<rhi::Backend> backend;
  inline static Ptr<rhi::Device> device;
  inline static Ptr<rhi::Texture> depthTexture;
  inline static Ptr<rhi::Sampler> sampler;
  inline static GraphicsPipeline currentPipeline;
  inline static Ptr<rhi::CommandBuffer> mainCommandBuffer;
  inline static Ptr<rhi::CommandBuffer> currentCommandBuffer;
  inline static Ptr<rhi::RenderPass> currentRenderPass;
  inline static bool isFrameStarted = true;
  inline static bool isPrevDepthEnabled = true;
  inline static bool isChangedRenderTarget = false;
  inline static uint32_t drawCallCountPerFrame = 0;
  inline static Array<rhi::ColorTargetInfo> colorTargets =
      Array<rhi::ColorTargetInfo>();
  inline static rhi::DepthStencilTargetInfo depthStencilInfo;
  inline static Array<rhi::ColorTargetInfo> currentColorTargets;
  inline static rhi::DepthStencilTargetInfo currentDepthStencilInfo;
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
