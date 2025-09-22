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
struct PxDrawable {
  PxDrawable(px::Allocator *allocator);
  px::Allocator *allocator;
  Array<px::BufferBinding> vertexBuffers;
  px::BufferBinding indexBuffer;
  Array<px::TextureSamplerBinding> textureSamplers;
  Ptr<Drawable> drawable;
};
struct PxVertexArray : public VertexArray {
  Ptr<px::Buffer> vertexBuffer;
  Ptr<px::Buffer> indexBuffer;
};
class GraphicsSystem {
public:
  static void initialize();
  static void shutdown();
  static void unload_data();
  static void render();
  static void SetCamera2D(const Camera2D &camera) {
    GraphicsSystem::camera2D = camera;
  }
  static Camera2D &GetCamera2D() { return camera2D; }
  static void SetCamera(const Camera &camera) {
    GraphicsSystem::camera = camera;
  }
  static Camera &GetCamera() { return camera; }
  static void Draw2D(const sinen::Draw2D &draw2D);
  static void Draw3D(const sinen::Draw3D &draw3D);
  static void DrawRect(const Rect &rect, const Color &color, float angle);
  static void DrawImage(const Texture &texture, const Rect &rect, float angle);
  static void DrawText(const std::string &text, const glm::vec2 &position,
                       const Color &color = Palette::white(),
                       float fontSize = 16.0f, float angle = 0.0f);
  static void DrawModel(const Model &model, const Transform &transform,
                        const Material &material);
  static void set_clear_color(const Color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }
  static Color get_clear_color() { return clearColor; }
  static void toggle_show_imgui() { showImGui = !showImGui; }
  static bool is_show_imgui() { return showImGui; }
  static void load_shader(const Shader &shaderinfo);
  static void unload_shader(const Shader &shaderinfo);
  static std::list<std::function<void()>> &get_imgui_function() {
    return m_imgui_function;
  }
  static void prepare_imgui();
  static void add_imgui_function(std::function<void()> function) {
    m_imgui_function.push_back(function);
  }
  static void *get_texture_id();

  static void bind_pipeline3d(const GraphicsPipeline3D &pipeline);
  static void bind_default_pipeline3d();
  static void bind_pipeline2d(const GraphicsPipeline2D &pipeline);
  static void bind_default_pipeline2d();
  static void set_uniform_data(uint32_t slot, const UniformData &data);

  static void SetRenderTarget(const RenderTexture &texture);
  static void Flush();
  static Texture ReadbackTexture(const RenderTexture &texture);

  static Model box;
  static Model sprite;

  static px::Allocator *GetAllocator() {
    static auto allocator = px::Paranoixa::CreateAllocator(0xffff);
    return allocator;
  }

  static px::Ptr<px::Device> GetDevice() { return device; }

private:
  static void setup_shapes();
  static Color clearColor;

  inline static Camera2D camera2D = glm::vec2(1280, 720);
  inline static Camera camera = []() {
    Camera c;
    c.LookAt(glm::vec3{0, -1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1});
    c.Perspective(90.f, Window::Size().x / Window::Size().y, .1f, 100.f);
    return c;
  }();
  ;
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> m_imgui_function;

  inline static px::Allocator *allocator = GetAllocator();
  inline static Ptr<px::Backend> backend;
  inline static Ptr<px::Device> device;
  inline static Ptr<px::Texture> depthTexture;
  inline static Ptr<px::Sampler> sampler;
  inline static GraphicsPipeline2D pipeline2D;
  inline static GraphicsPipeline3D pipeline3D;
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
      px::Array<px::ColorTargetInfo>(GetAllocator());
  inline static px::DepthStencilTargetInfo depthStencilInfo;
  inline static px::Array<px::ColorTargetInfo> currentColorTargets;
  inline static px::DepthStencilTargetInfo currentDepthStencilInfo;
  inline static px::HashMap<std::string, PxVertexArray> vertexArrays =
      px::HashMap<std::string, PxVertexArray>(GetAllocator());
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
