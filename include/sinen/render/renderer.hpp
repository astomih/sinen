#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../color/color.hpp"
#include "../drawable/drawable.hpp"
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"
#include "../model/model.hpp"
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"
#include "../vertex/vertex_array.hpp"

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
   * @brief Draw UI drawable object
   *
   * @param draw_object
   */
  static void drawui(const std::shared_ptr<Drawable> draw_object);
  /**
   * @brief Draw 3d drawable object
   *
   * @param draw_object
   */
  static void draw3d(const std::shared_ptr<Drawable> draw_object);
  /**
   * @brief Add vertex array
   *
   * @param vArray
   * @param name
   */
  static void add_vertex_array(const VertexArray &vArray,
                               std::string_view name);
  /**
   * @brief Update vertex array
   *
   * @param vArray
   * @param name
   */
  static void update_vertex_array(const VertexArray &vArray,
                                  std::string_view name);
  static void add_model(const Model &_model);
  static void update_model(const Model &_model);
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
  static void set_light_look_at(const Vector3 &position, const Vector3 &target,
                                const Vector3 &up);
  static void set_light_ortho(float left, float right, float bottom, float top,
                              float near, float far);
  static void *get_texture_id();
  static bool offscreen_rendering;
  static matrix4 render_texture_user_data;
  static void at_render_texture_user_data(int index, float value) {
    render_texture_user_data.mat.m16[index] = value;
  }
  static matrix4 light_view;
  static matrix4 light_projection;
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
