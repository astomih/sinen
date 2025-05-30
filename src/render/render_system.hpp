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
class RendererSystem {
public:
  static void initialize();
  static void shutdown();
  static void unload_data();
  static void render();
  static void draw2d(const std::shared_ptr<Drawable> draw_object);
  static void draw3d(const std::shared_ptr<Drawable> draw_object);
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

  static std::shared_ptr<class PxRenderer> GetPxRenderer() {
    return pxRenderer;
  }

  static void begin_pipeline3d(const RenderPipeline3D &pipeline);
  static void end_pipeline3d();
  static void begin_pipeline2d(const RenderPipeline2D &pipeline);
  static void end_pipeline2d();
  static void set_uniform_data(uint32_t slot, const UniformData &data);

  static void begin_render_texture2d(const RenderTexture &texture);
  static void begin_render_texture3d(const RenderTexture &texture);
  static void end_render_texture(const RenderTexture &texture, Texture &out);

  static Model box;
  static Model sprite;

private:
  static std::shared_ptr<class PxRenderer> pxRenderer;
  static void setup_shapes();
  static Color clearColor;
  // Renderer
  static bool showImGui;
  static std::list<std::function<void()>> m_imgui_function;
};
} // namespace sinen
#endif // !SINEN_RENDER_SYSTEM_HPP
