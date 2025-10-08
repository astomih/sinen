#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
// std
#include <functional>
#include <list>
#include <memory>
// internal
#include <asset/asset.hpp>
#include <core/allocator/pool_allocator.hpp>
#include <graphics/drawable/drawable.hpp>
#include <graphics/graphics.hpp>
#include <graphics/uniform_data.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/math.hpp>
#include <platform/window/window.hpp>

namespace sinen {
template <typename T> using Ptr = px::Ptr<T>;
template <typename T> using Array = px::Array<T>;
class GraphicsSystem {
public:
  static void initialize();
  static void shutdown();
  static void render();
  static void setCamera2D(const Camera2D &camera) {
    GraphicsSystem::camera2D = camera;
  }
  static Camera2D &getCamera2D() { return camera2D; }
  static void setCamera(const Camera &camera) {
    GraphicsSystem::camera = camera;
  }
  static Camera &getCamera() { return camera; }
  static void drawBase2D(const sinen::Draw2D &draw2D);
  static void drawBase3D(const sinen::Draw3D &draw3D);
  static void drawRect(const Rect &rect, const Color &color, float angle);
  static void drawImage(const Texture &texture, const Rect &rect, float angle);
  static void drawText(const std::string &text, const glm::vec2 &position,
                       const Color &color = Palette::white(),
                       float fontSize = 16.0f, float angle = 0.0f);
  static void drawModel(const Model &model, const Transform &transform,
                        const Material &material);
  static void setClearColor(const Color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }
  static Color getClearColor() { return clearColor; }
  static void toggleShowImGui() { showImGui = !showImGui; }
  static bool isShowImGui() { return showImGui; }
  static void loadShader(const Shader &shaderinfo);
  static void unloadShader(const Shader &shaderinfo);
  static std::list<std::function<void()>> &getImGuiFunction() {
    return imguiFunctions;
  }
  static void addImGuiFunction(std::function<void()> function) {
    imguiFunctions.push_back(function);
  }
  static void bindPipeline3D(const GraphicsPipeline3D &pipeline);
  static void bindDefaultPipeline3D();
  static void bindPipeline2D(const GraphicsPipeline2D &pipeline);
  static void bindDefaultPipeline2D();
  static void setUniformData(uint32_t slot, const UniformData &data);

  static void setRenderTarget(const RenderTexture &texture);
  static void flush();
  static bool readbackTexture(const RenderTexture &texture, Texture &out);

  static Model box;
  static Model sprite;

  static px::Allocator *getAllocator() {
    static auto allocator = px::Paranoixa::CreateAllocator(0xffff);
    return allocator;
  }

  static px::Ptr<px::Device> getDevice() { return device; }

private:
  static void setupShapes();
  static Color clearColor;

  inline static Camera camera;
  inline static Camera2D camera2D;
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> imguiFunctions;

  inline static px::Allocator *allocator = getAllocator();
  inline static Ptr<px::Backend> backend;
  inline static Ptr<px::Device> device;
  inline static Ptr<px::Texture> depthTexture;
  inline static Ptr<px::Sampler> sampler;
  inline static GraphicsPipeline2D pipeline2D;
  inline static GraphicsPipeline3D pipeline3D;
  inline static GraphicsPipeline3D pipelineInstanced3D;
  inline static GraphicsPipeline2D currentPipeline2D;
  inline static GraphicsPipeline3D currentPipeline3D;
  inline static px::Ptr<px::CommandBuffer> mainCommandBuffer;
  inline static px::Ptr<px::CommandBuffer> currentCommandBuffer;
  inline static px::Ptr<px::RenderPass> currentRenderPass;
  inline static bool isFrameStarted = true;
  inline static bool isDraw2D = true;
  inline static bool isDefaultPipeline = true;
  inline static uint32_t objectCount = 0;
  inline static px::Array<px::ColorTargetInfo> colorTargets =
      px::Array<px::ColorTargetInfo>(getAllocator());
  inline static px::DepthStencilTargetInfo depthStencilInfo;
  inline static px::Array<px::ColorTargetInfo> currentColorTargets;
  inline static px::DepthStencilTargetInfo currentDepthStencilInfo;
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
