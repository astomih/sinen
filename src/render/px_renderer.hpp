#ifndef SINEN_PX_RENDERER_HPP
#define SINEN_PX_RENDERER_HPP

#include <drawable/drawable.hpp>
#include <model/model.hpp>
#include <paranoixa/paranoixa.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
class PxRenderer {
public:
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
  px::AllocatorPtr allocator;
  px::Ptr<px::Backend> backend;
  px::Ptr<px::Device> device;
};
} // namespace sinen

#endif // !SINEN_PX_RENDERER_HPP
