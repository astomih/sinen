#ifndef SINEN_PBR_RENDERER_HPP
#define SINEN_PBR_RENDERER_HPP

#include <graphics/render_pass.hpp>
#include <optional>
#include <renderer/global_illumination.hpp>
#include <renderer/light.hpp>

namespace sinen {

class PBRRenderer {
public:
  struct Environment {
    Ptr<Texture> brdfLut;
    Ptr<Texture> irradiance;
    Ptr<Texture> prefiltered;
  };

  static constexpr const char *metaTableName() { return "sn.PBRRenderer"; }

  // Replace the direct light with a copy of the supplied light.
  void setLight(const Light &light);
  void setPointLight(const Vec3 &position, float intensity,
                     const Color &color = Color(1.0f));
  void setFillLight(const Light &light);
  void clearFillLight();
  // Positive exposure enables tone mapping and sRGB output for an SDR target.
  void setExposure(float value);
  // Use the supplied lighting textures when all three are ready.
  void setEnvironment(const Environment &environment);
  void clearEnvironment();
  // Attach a diffuse volume. Real-time textures update in place; reattach only
  // after reinitializing or replacing a CPU bake.
  void setGlobalIllumination(const GlobalIllumination &volume,
                             float intensity = 1.0f);
  void clearGlobalIllumination();

  // Uses the model's glTF-style material textures and the pass's camera.
  void drawModel(Render3DPass &pass, const Model &model,
                 const Transform &transform);

private:
  Environment environment;
  Light light = PointLight(Vec3(1.0f, 1.0f, 1.0f));
  std::optional<Light> fillLight;
  float exposure = 0.0f;
  Ptr<Texture> giTexture;
  Vec3 giMinimum, giExtent, giCounts;
  float giIntensity = 1.0f;
  Ptr<GraphicsPipeline> directPipeline;
  Ptr<GraphicsPipeline> iblPipeline;
  Ptr<Texture> white;
  Ptr<Texture> black;
  Ptr<Texture> defaultMetallicRoughness;
};

} // namespace sinen

#endif
