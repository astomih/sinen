#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../color/color.hpp"
#include "../instancing/instancing.hpp"
#include "../math/math.hpp"
#include "../math/matrix4.hpp"
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../model/model.hpp"
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"
#include "../texture/texture_type.hpp"
#include "../vertex/vertex_array.hpp"
#include "graphics_api.hpp"

namespace sinen {
/**
 * @brief Renderer class
 *
 */
class renderer {
public:
  /**
   * @brief Get the graphics api object
   *
   * @return graphics_api
   */
  static graphics_api get_graphics_api();
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
  static void draw2d(const std::shared_ptr<drawable> draw_object);
  /**
   * @brief Draw 3d drawable object
   *
   * @param draw_object
   */
  static void draw3d(const std::shared_ptr<drawable> draw_object);
  /**
   * @brief Add vertex array
   *
   * @param vArray
   * @param name
   */
  static void add_vertex_array(const vertex_array &vArray,
                               std::string_view name);
  /**
   * @brief Update vertex array
   *
   * @param vArray
   * @param name
   */
  static void update_vertex_array(const vertex_array &vArray,
                                  std::string_view name);
  static void add_model(const model &_model);
  static void update_model(const model &_model);
  /**
   * @brief Draw instancing
   *
   * @param _instancing
   */
  static void add_instancing(const instancing &_instancing);
  /**
   * @brief Set the clear color object
   *
   * @param color
   */
  static void set_clear_color(const color &color);
  /**
   * @brief Get the clear color object
   *
   * @return color
   */
  static color clear_color();
  /**
   * @brief Set the skybox object
   *
   * @param _skybox_texture
   */
  static void set_skybox(texture _skybox_texture);
  /**
   * @brief Get the skybox object
   *
   * @return texture
   */
  static texture skybox();
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
   * @brief Load shader
   *
   * @param shaderinfo
   */
  static void load_shader(const shader &shaderinfo);
  /**
   * @brief Unload shader
   *
   * @param shaderinfo
   */
  static void unload_shader(const shader &shaderinfo);
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
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
