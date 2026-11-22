#ifndef SINEN_GLOBAL_ILLUMINATION_HPP
#define SINEN_GLOBAL_ILLUMINATION_HPP

#include <graphics/model/model.hpp>
#include <math/transform/transform.hpp>
#include <optional>
#include <renderer/light.hpp>
#include <vector>

namespace sinen {

// Diffuse irradiance volume with GPU real-time and CPU baking modes.
// No hardware ray tracing is required.
class GlobalIllumination {
public:
  static constexpr const char *metaTableName() {
    return "sn.GlobalIllumination";
  }
  void clear();
  // Includes vertex colors and the mean texture color. Geometry is opaque and
  // two-sided; optional simplification omits alpha-masked parts for a GI proxy.
  void addModel(const Model &model, const Transform &transform,
                const Color &albedo = Color(1.0f),
                const Color &emission = Color(0.0f), UInt32 triangleBudget = 0);
  void setLight(const Light &value);
  void clearLight();
  void setSkyColor(const Color &value);
  // GPU software ray tracing. Call update before rendering each frame. Uses
  // 64 rays per probe per frame and 1..4 diffuse hits, with temporal filtering.
  // Returns false on unsupported backends (SDLGPU), invalid bounds, or failure.
  bool initializeRealtime(const Vec3 &minimum, const Vec3 &maximum,
                          UInt32 countX = 8, UInt32 countY = 6,
                          UInt32 countZ = 8, UInt32 bounces = 2);
  // Replace scene geometry for this frame while retaining lighting history.
  // Follow with addModel calls for all participating objects, then update.
  void beginFrame();
  bool update(float historyWeight = 0.85f);
  bool isRealtime() const { return realtime != nullptr; }
  // Inclusive bounds, 2..32 probes per axis, 32..65536 rays, 1..8 diffuse hits.
  // Synchronous deterministic bake. Invalid settings leave the previous bake.
  bool bake(const Vec3 &minimum, const Vec3 &maximum, UInt32 countX = 8,
            UInt32 countY = 6, UInt32 countZ = 8, UInt32 rays = 512,
            UInt32 bounces = 3);
  bool isBaked() const { return !irradiance.empty(); }
  Vec3 sampleIrradiance(const Vec3 &position, const Vec3 &normal) const;
  Ptr<Texture> getTexture() const;
  Vec3 getMinimum() const { return minimum; }
  Vec3 getMaximum() const { return maximum; }
  Vec3 getProbeCounts() const;

private:
  struct RealtimeState;
  std::shared_ptr<RealtimeState> realtime;
  struct Triangle {
    Vec3 a, b, c, albedo, emission;
  };
  void invalidate();
  std::vector<Triangle> triangles;
  std::optional<Light> light;
  Vec3 sky;
  Vec3 minimum, maximum;
  UInt32 countX = 0, countY = 0, countZ = 0;
  // Six cosine-convolved lobes per probe: +X, -X, +Y, -Y, +Z, -Z.
  std::vector<Vec3> irradiance;
  mutable Ptr<Texture> texture;
};

} // namespace sinen
#endif
