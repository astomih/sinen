#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
// std
#include <functional>
#include <list>
#include <memory>
// internal
#include <asset/asset.hpp>
#include <core/allocator/pool_allocator.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/math.hpp>
#include <render/drawable/drawable.hpp>
#include <render/renderer.hpp>

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
class RendererSystem {
public:
  static void initialize();
  static void shutdown();
  static void unload_data();
  static void render();
  static void draw2d(const std::shared_ptr<Drawable> &drawObject);
  static void draw3d(const std::shared_ptr<Drawable> &drawObject);
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

  static void bind_pipeline3d(const RenderPipeline3D &pipeline);
  static void bind_default_pipeline3d();
  static void bind_pipeline2d(const RenderPipeline2D &pipeline);
  static void bind_default_pipeline2d();
  static void set_uniform_data(uint32_t slot, const UniformData &data);

  static void begin_target2d(const RenderTexture &texture);
  static void begin_target3d(const RenderTexture &texture);
  static void end_target(const RenderTexture &texture, Texture &out);

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
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> m_imgui_function;

  inline static px::Allocator *allocator = GetAllocator();
  inline static Ptr<px::Backend> backend;
  inline static Ptr<px::Device> device;
  inline static Ptr<px::Texture> depthTexture;
  inline static Ptr<px::Sampler> sampler;
  inline static RenderPipeline2D pipeline2D;
  inline static RenderPipeline3D pipeline3D;
  inline static RenderPipeline2D currentPipeline2D;
  inline static RenderPipeline3D currentPipeline3D;
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
