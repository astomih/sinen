#ifndef SINEN_PBR_RENDERER_HPP
#define SINEN_PBR_RENDERER_HPP

#include <graphics/render_pass.hpp>

namespace sinen {

class PBRRenderer {
public:
  struct Environment {
    Ptr<Texture> brdfLut;
    Ptr<Texture> irradiance;
    Ptr<Texture> prefiltered;
  };

  static constexpr const char *metaTableName() { return "sn.PBRRenderer"; }

  void setPointLight(const Vec3 &position, float intensity,
                     const Color &color = Color(1.0f));
  // Use the supplied lighting textures when all three are ready.
  void setEnvironment(const Environment &environment);
  void clearEnvironment();

  // Uses the model's glTF-style material textures and the pass's camera.
  void drawModel(Render3DPass &pass, const Model &model,
                 const Transform &transform);

private:
  Environment environment;
  Vec3 lightPosition{1.0f, 1.0f, 1.0f};
  Color lightColor{1.0f};
  float lightIntensity = 4.0f;
  Ptr<GraphicsPipeline> directPipeline;
  Ptr<GraphicsPipeline> iblPipeline;
  Ptr<Texture> white;
  Ptr<Texture> black;
  Ptr<Texture> defaultMetallicRoughness;
};

} // namespace sinen

#endif
