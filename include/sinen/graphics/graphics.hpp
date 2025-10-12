#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../asset/model/mesh.hpp"
#include "../asset/model/model.hpp"
#include "../asset/shader/shader.hpp"
#include "../asset/texture/render_texture.hpp"
#include "../asset/texture/texture.hpp"
#include "../math/color/color.hpp"
#include "../math/color/palette.hpp"
#include "../physics/primitive2.hpp"
#include "../physics/primitive3.hpp"
#include "camera/camera.hpp"
#include "camera/camera2d.hpp"
#include "drawable/drawable.hpp"
#include "graphics_pipeline.hpp"

namespace sinen {
/**
 * @brief Renderer class
 *
 */
class Graphics {
public:
  static void draw2D(const Draw2D &draw2d);
  static void draw3D(const Draw3D &draw3D);

  static void drawRect(const Rect &rect, const Color &color, float angle);
  static void drawRect(const Rect &rect, const Color &color) {
    drawRect(rect, color, 0.0f);
  }
  static void drawImage(const Texture &texture, const Rect &rect, float angle);
  static void drawImage(const Texture &texture, const Rect &rect) {
    drawImage(texture, rect, 0.0f);
  }
  static void drawText(const std::string &text, const glm::vec2 &position,
                       const Color &color, float fontSize, float angle);
  static void drawText(const std::string &text, const glm::vec2 &position,
                       const Color &color, float fontSize) {
    drawText(text, position, color, fontSize, 0.0f);
  }
  static void drawText(const std::string &text, const glm::vec2 &position,
                       const Color &color) {
    drawText(text, position, color, 16.0f, 0.0f);
  }
  static void drawText(const std::string &text, const glm::vec2 &position) {
    drawText(text, position, Palette::white(), 16.0f, 0.0f);
  }

  static void drawModel(const Model &model, const Transform &transform,
                        const Material &material);

  static void drawModelInstanced(const Model &model,
                                 const std::vector<Transform> &transforms,
                                 const Material &material);

  /**
   * @brief Set the clear color object
   *
   * @param color
   */
  static void setClearColor(const Color &color);
  /**
   * @brief Get the clear color object
   *
   * @return color
   */
  static Color getClearColor();
  /**
   * @brief Toggle show imgui
   *
   */
  static void toggleShowImGui();
  /**
   * @brief Is show ImGui
   *
   * @return true showing
   * @return false not showing
   */
  static bool isShowImGui();
  /**
   * @brief Get the imgui function object
   *
   * @return std::list<std::function<void()>>&
   */
  static std::list<std::function<void()>> &getImGuiFunction();
  /**
   * @brief Get the ImGui function object
   *
   * @param function
   */
  static void addImGuiFunction(std::function<void()> function);

  static void setCamera2D(const Camera2D &camera);
  static Camera2D &getCamera2D();
  static void setCamera(const Camera &camera);
  static Camera &getCamera();
  static void bindPipeline3D(const GraphicsPipeline3D &pipeline);
  static void bindDefaultPipeline3D();
  static void bindPipeline2D(const GraphicsPipeline2D &pipeline);
  static void bindDefaultPipeline2D();
  static void setUniformData(uint32_t slot, const UniformData &data);

  static void setRenderTarget(const RenderTexture &texture);
  static void flush();
  static bool readbackTexture(const RenderTexture &texture, Texture &out);
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
