#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../asset/model/model.hpp"
#include "../asset/model/vertex_array.hpp"
#include "../asset/shader/shader.hpp"
#include "../asset/texture/render_texture.hpp"
#include "../asset/texture/texture.hpp"
#include "../math/color/color.hpp"
#include "drawable/drawable.hpp"
#include "render_pipeline.hpp"

namespace sinen {
/**
 * @brief Renderer class
 *
 */
class Renderer {
public:
  /**
   * @brief Unload all resources
   *
   */
  static void unload_data();
  /**
   * @brief Render
   *
   */
  static void render();
  /**
   * @brief Draw 2d drawable object
   *
   * @param draw_object
   */
  static void draw2d(const std::shared_ptr<Drawable> draw_object);
  /**
   * @brief Draw 3d drawable object
   *
   * @param draw_object
   */
  static void draw3d(const std::shared_ptr<Drawable> draw_object);
  /**
   * @brief Set the clear color object
   *
   * @param color
   */
  static void set_clear_color(const Color &color);
  /**
   * @brief Get the clear color object
   *
   * @return color
   */
  static Color clear_color();
  /**
   * @brief Toggle show imgui
   *
   */
  static void toggle_show_imgui();
  /**
   * @brief Is show ImGui
   *
   * @return true showing
   * @return false not showing
   */
  static bool is_show_imgui();
  /**
   * @brief Get the imgui function object
   *
   * @return std::list<std::function<void()>>&
   */
  static std::list<std::function<void()>> &get_imgui_function();
  /**
   * @brief Get the ImGui function object
   *
   * @param function
   */
  static void add_imgui_function(std::function<void()> function);
  static void *get_texture_id();
  static bool offscreen_rendering;
  static glm::mat4 render_texture_user_data;
  static void at_render_texture_user_data(int index, float value) {}

  static void begin_pipeline3d(const RenderPipeline3D &pipeline);
  static void end_pipeline3d();
  static void begin_pipeline2d(const RenderPipeline2D &pipeline);
  static void end_pipeline2d();
  static void set_uniform_data(uint32_t slot, const UniformData &data);

  static void begin_render_texture2d(const RenderTexture &texture);
  static void begin_render_texture3d(const RenderTexture &texture);
  static void end_render_texture(const RenderTexture &texture, Texture &out);
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
