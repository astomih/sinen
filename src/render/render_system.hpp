#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
#include <color/color.hpp>
#include <color/palette.hpp>
#include <instancing/instancing.hpp>
#include <list>
#include <math/math.hpp>
#include <math/matrix4.hpp>
#include <math/vector2.hpp>
#include <math/vector3.hpp>
#include <memory>
#include <render/graphics_api.hpp>
#include <shader/shader.hpp>
#include <texture/texture.hpp>
#include <texture/texture_type.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
class render_system {
public:
  render_system();
  ~render_system();
  void initialize(graphics_api api);
  void shutdown();
  graphics_api get_graphics_api();

  void unload_data();

  void render();

  void draw2d(const std::shared_ptr<draw_object> draw_object);
  void draw3d(const std::shared_ptr<draw_object> draw_object);

  void add_vertex_array(const vertex_array &vArray, std::string_view name);
  void update_vertex_array(const vertex_array &vArray, std::string_view name);

  void add_instancing(const instancing &_instancing);

  void set_clear_color(const color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }

  color get_clear_color() { return this->clearColor; }

  void set_skybox_texture(texture _skybox_texture) {
    m_skybox_texture = _skybox_texture;
  }

  texture get_skybox_texture() { return m_skybox_texture; }

  void toggle_show_imgui() { showImGui = !showImGui; }
  bool is_show_imgui() { return showImGui; }

  void load_shader(const shader &shaderinfo);
  void unload_shader(const shader &shaderinfo);

  std::list<std::function<void()>> &get_imgui_function() {
    return m_imgui_function;
  }

  void add_imgui_function(std::function<void()> function) {
    m_imgui_function.push_back(function);
  }

private:
  std::unique_ptr<class gl_renderer> m_gl_renderer;
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::unique_ptr<class vk_renderer> m_vk_renderer;
#endif
  void setup_shapes();
  color clearColor = palette::Black;

  // Window
  std::shared_ptr<class window> mWindow;
  // Renderer
  graphics_api RendererAPI;
  bool showImGui;
  std::list<std::function<void()>> m_imgui_function;
  texture m_skybox_texture;
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
