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
#include "../model/vertex_array.hpp"
#include "../shader/shader.hpp"
#include "../texture/render_texture.hpp"
#include "../texture/texture.hpp"
#include "render/render_pipeline.hpp"
#include "render_pipeline.hpp"

namespace sinen {
struct UniformData {
  UniformData() : data() {}
  void add(float value) { data.push_back(value); }
  void add_matrix(const matrix4 &matrix) {
    data.insert(data.end(), matrix.mat.m16, matrix.mat.m16 + 16);
  }
  void add_matrices(const std::vector<matrix4> &matrices) {
    for (auto &matrix : matrices) {
      data.insert(data.end(), matrix.mat.m16, matrix.mat.m16 + 16);
    }
  }
  void add_vector3(const Vector3 &vector) {
    data.push_back(vector.x);
    data.push_back(vector.y);
    data.push_back(vector.z);
  }
  void change(float value, int index) { data[index] = value; }
  std::vector<float> data;
};
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
  static matrix4 render_texture_user_data;
  static void at_render_texture_user_data(int index, float value) {
    render_texture_user_data.mat.m16[index] = value;
  }

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
