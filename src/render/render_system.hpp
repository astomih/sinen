#ifndef SINEN_RENDER_SYSTEM_HPP
#define SINEN_RENDER_SYSTEM_HPP
// std
#include <functional>
#include <list>
#include <memory>
// internal
#include <allocator/pool_allocator.hpp>
#include <color/color.hpp>
#include <color/palette.hpp>
#include <drawable/drawable.hpp>
#include <math/math.hpp>
#include <math/matrix4.hpp>
#include <math/vector2.hpp>
#include <math/vector3.hpp>
#include <model/model.hpp>
#include <shader/shader.hpp>
#include <texture/texture.hpp>
#include <vertex/vertex_array.hpp>

namespace sinen {
class render_system {
public:
  static void initialize();
  static void shutdown();
  static void unload_data();
  static void render();
  static void draw2d(const std::shared_ptr<drawable> draw_object);
  static void drawui(const std::shared_ptr<drawable> draw_object);
  static void draw3d(const std::shared_ptr<drawable> draw_object);
  static void add_queue_2d(const std::shared_ptr<drawable> draw_object);
  static void add_queue_3d(const std::shared_ptr<drawable> draw_object);
  static void remove_queue_2d(const std::shared_ptr<drawable> draw_object);
  static void remove_queue_3d(const std::shared_ptr<drawable> draw_object);
  static void add_vertex_array(const vertex_array &vArray,
                               std::string_view name);
  static void update_vertex_array(const vertex_array &vArray,
                                  std::string_view name);
  static void add_model(const model &m);
  static void update_model(const model &m);
  static void set_clear_color(const color &color) {
    if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
      clearColor = color;
  }
  static color get_clear_color() { return clearColor; }
  static void set_skybox_texture(texture _skybox_texture) {
    m_skybox_texture = _skybox_texture;
  }
  static texture get_skybox_texture() { return m_skybox_texture; }
  static void toggle_show_imgui() { showImGui = !showImGui; }
  static bool is_show_imgui() { return showImGui; }
  static void load_shader(const shader &shaderinfo);
  static void unload_shader(const shader &shaderinfo);
  static std::list<std::function<void()>> &get_imgui_function() {
    return m_imgui_function;
  }
  static void add_imgui_function(std::function<void()> function) {
    m_imgui_function.push_back(function);
  }
  static void *get_texture_id();

private:
  static class vk_renderer m_vk_renderer;
  static void setup_shapes();
  static color clearColor;
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> m_imgui_function;
  static texture m_skybox_texture;
  static std::vector<std::shared_ptr<drawable>> m_drawable_2d;
  static std::vector<std::shared_ptr<drawable>> m_drawable_3d;
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
