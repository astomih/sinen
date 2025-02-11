#ifndef SINEN_PX_RENDERER_HPP
#define SINEN_PX_RENDERER_HPP

#include "paranoixa/define.hpp"
#include <drawable/drawable.hpp>
#include <model/model.hpp>
#include <paranoixa/paranoixa.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
template <typename T> using Ptr = px::Ptr<T>;
template <typename T> using Array = px::Array<T>;
struct PxDrawable {
  PxDrawable(px::AllocatorPtr allocator);
  px::AllocatorPtr allocator;
  Array<px::BufferBinding> vertexBuffers;
  px::BufferBinding indexBuffer;
  Array<px::TextureSamplerBinding> textureSamplers;
  Drawable drawable;
};
struct PxVertexArray : public VertexArray {
  Ptr<px::Buffer> vertexBuffer;
  Ptr<px::Buffer> indexBuffer;
};
class PxRenderer {
public:
  PxRenderer(px::AllocatorPtr allocator);
  void initialize();
  void shutdown();
  void unload_data();
  void render();
  void draw2d(const std::shared_ptr<Drawable> draw_object);
  void drawui(const std::shared_ptr<Drawable> draw_object);
  void draw3d(const std::shared_ptr<Drawable> draw_object);
  void add_vertex_array(const VertexArray &vArray, std::string_view name);
  void update_vertex_array(const VertexArray &vArray, std::string_view name);
  void add_model(const Model &m);
  void update_model(const Model &m);
  void load_shader(const Shader &shaderinfo);
  void unload_shader(const Shader &shaderinfo);
  void prepare_imgui();
  void *get_texture_id();

private:
  Ptr<px::Texture> CreateNativeTexture(const HandleT &handle);
  px::AllocatorPtr allocator;
  Ptr<px::Backend> backend;
  Ptr<px::Device> device;

  Ptr<px::GraphicsPipeline> pipelineRenderTexture;
  Ptr<px::GraphicsPipeline> pipelinePresentTexture;
  Ptr<px::GraphicsPipeline> pipelineOpaque;
  Ptr<px::GraphicsPipeline> pipelineAlpha;
  Ptr<px::GraphicsPipeline> pipeline2D;
  Ptr<px::GraphicsPipeline> pipelineInstancingOpaque;
  Ptr<px::GraphicsPipeline> pipelineInstancingAlpha;
  Ptr<px::GraphicsPipeline> pipelineInstancing2D;
  Ptr<px::GraphicsPipeline> pipelineUI;

  px::Array<PxDrawable> drawables2D;
  px::HashMap<std::string, PxVertexArray> vertexArrays;
  px::HashMap<HandleT, px::TextureSamplerBinding> textureSamplers;
};
} // namespace sinen

#endif // !SINEN_PX_RENDERER_HPP
