#ifndef SINEN_RENDER_RENDERER_HPP
#define SINEN_RENDER_RENDERER_HPP
#include <functional>
#include <list>

#include <asset/Font/font.hpp>
#include <asset/model/model.hpp>
#include <asset/shader/shader.hpp>
#include <asset/texture/render_texture.hpp>
#include <asset/texture/texture.hpp>
#include <core/buffer/buffer.hpp>
#include <geometry/mesh.hpp>
#include <geometry/rect.hpp>
#include <graphics/camera/camera.hpp>
#include <graphics/camera/camera2d.hpp>
#include <graphics/drawable/drawable.hpp>
#include <graphics/graphics_pipeline.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/vector.hpp>
#include <physics/primitive2.hpp>
#include <physics/primitive3.hpp>


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
  static void drawText(StringView text, const Font &font, const Vec2 &position,
                       const Color &color, float textSize, float angle);
  static void drawText(StringView text, const Font &font, const Vec2 &position,
                       const Color &color) {
    drawText(text, font, position, color, 32.f, 0.0f);
  }
  static void drawText(StringView text, const Font &font,
                       const Vec2 &position) {
    drawText(text, font, position, Palette::white(), 32.f, 0.0f);
  }

  static void drawCubemap(const Cubemap &cubemap);

  static void drawModel(const Model &model, const Transform &transform,
                        const Material &material);

  static void drawModelInstanced(const Model &model,
                                 const Array<Transform> &transforms,
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
  static void bindPipeline(const GraphicsPipeline &pipeline);
  static void setUniformBuffer(uint32_t slot, const Buffer &data);

  static void setRenderTarget(const RenderTexture &texture);
  static void flush();
  static bool readbackTexture(const RenderTexture &texture, Texture &out);
};

} // namespace sinen
#endif // !SINEN_RENDERER_HPP
