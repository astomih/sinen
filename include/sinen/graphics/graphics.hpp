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
#include "drawable/drawable_wrapper.hpp"
#include "graphics_pipeline.hpp"

namespace sinen {
/**
 * @brief Renderer class
 *
 */
class Graphics {
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

  static void Draw2D(const Draw2D &draw2d);
  static void Draw3D(const Draw3D &draw3d);
  /**
   * @brief Set the clear color object
   *
   * @param color
   */
  static void SetClearColor(const Color &color);
  /**
   * @brief Get the clear color object
   *
   * @return color
   */
  static Color GetClearColor();
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

  static void BindPipeline3D(const GraphicsPipeline3D &pipeline);
  static void BindDefaultPipeline3D();
  static void BindPipeline2D(const GraphicsPipeline2D &pipeline);
  static void BindDefaultPipeline2D();
  static void SetUniformData(uint32_t slot, const UniformData &data);

  static void BeginTarget2D(const RenderTexture &texture);
  static void BeginTarget3D(const RenderTexture &texture);
  static void EndTarget(const RenderTexture &texture, Texture &out);
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
