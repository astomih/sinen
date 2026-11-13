#ifndef SINEN_NPR_RENDERER_HPP
#define SINEN_NPR_RENDERER_HPP

#include <graphics/render_pass.hpp>
#include <renderer/light.hpp>

namespace sinen {

class NPRRenderer {
public:
  static constexpr const char *metaTableName() { return "sn.NPRRenderer"; }

  // Replace the direct light with a copy of the supplied light.
  void setLight(const Light &light);
  // World-space direction from the surface toward the light.
  void setLightDirection(const Vec3 &direction);
  void setShadowColor(const Color &color);
  void setBands(UInt32 bands);

  void drawModel(Render3DPass &pass, const Model &model,
                 const Transform &transform);

private:
  Light light = DirectionalLight();
  Color shadowColor{0.25f, 0.30f, 0.45f, 1.0f};
  UInt32 bands = 3;
  Ptr<GraphicsPipeline> pipeline;
};

} // namespace sinen

#endif
