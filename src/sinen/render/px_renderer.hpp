#ifndef SINEN_PX_RENDERER_HPP
#define SINEN_PX_RENDERER_HPP

#include <cstdint>
#include <drawable/drawable.hpp>
#include <model/model.hpp>
#include <model/vertex_array.hpp>
#include <paranoixa/paranoixa.hpp>
#include <render/render_pipeline.hpp>
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
class PxRenderer {
public:
  PxRenderer(px::Allocator *allocator);
  void initialize();
  void shutdown();
  void unload_data();
  void render();
  void draw2d(const std::shared_ptr<Drawable> draw_object);
  void draw3d(const std::shared_ptr<Drawable> draw_object);
  void load_shader(const Shader &shaderinfo);
  void unload_shader(const Shader &shaderinfo);
  void prepare_imgui();
  void *get_texture_id();

  px::Allocator *GetAllocator() { return allocator; }
  px::Ptr<px::Device> GetDevice() { return device; }

private:
  px::Allocator *allocator;
  Ptr<px::Backend> backend;
  Ptr<px::Device> device;

  Ptr<px::Texture> depthTexture;
  Ptr<px::Sampler> sampler;
  RenderPipeline2D pipeline2D;
  RenderPipeline3D pipeline3D;

  px::Ptr<px::CommandBuffer> currentCommandBuffer;
  px::Ptr<px::RenderPass> currentRenderPass;
  bool isFrameStarted = true;
  bool isDraw2D = true;
  uint32_t objectCount = 0;
  px::Array<px::ColorTargetInfo> colorTargets;
  px::DepthStencilTargetInfo depthStencilInfo;
  px::HashMap<std::string, PxVertexArray> vertexArrays;
};
} // namespace sinen

#endif // !SINEN_PX_RENDERER_HPP
