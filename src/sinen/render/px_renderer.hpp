#ifndef SINEN_PX_RENDERER_HPP
#define SINEN_PX_RENDERER_HPP

#include "render/renderer.hpp"
#include <asset/model/model.hpp>
#include <asset/model/vertex_array.hpp>
#include <asset/texture/render_texture.hpp>
#include <cstdint>
#include <paranoixa/paranoixa.hpp>
#include <render/drawable/drawable.hpp>
#include <render/render_pipeline.hpp>

#include <optional>
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

  void begin_pipeline3d(const RenderPipeline3D &pipeline);
  void end_pipeline3d();
  void begin_pipeline2d(const RenderPipeline2D &pipeline);
  void end_pipeline2d();
  void set_uniform_data(uint32_t slot, const UniformData &data);

  void begin_render_texture2d(const RenderTexture &texture);
  void begin_render_texture3d(const RenderTexture &texture);
  void end_render_texture(const RenderTexture &texture, Texture &out);

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

  RenderPipeline2D currentPipeline2D;
  RenderPipeline3D currentPipeline3D;

  px::Ptr<px::CommandBuffer> mainCommandBuffer;
  px::Ptr<px::CommandBuffer> currentCommandBuffer;
  px::Ptr<px::RenderPass> currentRenderPass;
  bool isFrameStarted = true;
  bool isDraw2D = true;
  bool isDefaultPipeline = true;
  uint32_t objectCount = 0;
  px::Array<px::ColorTargetInfo> colorTargets;
  px::DepthStencilTargetInfo depthStencilInfo;

  px::Array<px::ColorTargetInfo> currentColorTargets;
  px::DepthStencilTargetInfo currentDepthStencilInfo;

  px::HashMap<std::string, PxVertexArray> vertexArrays;
};
} // namespace sinen

#endif // !SINEN_PX_RENDERER_HPP
