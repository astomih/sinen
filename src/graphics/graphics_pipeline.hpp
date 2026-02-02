#ifndef SINEN_RENDER_PIPELINE_HPP
#define SINEN_RENDER_PIPELINE_HPP
#include "shader/shader.hpp"
#include <gpu/gpu_graphics_pipeline.hpp>

#include <bitset>

namespace sinen {

class GraphicsPipeline {
public:
  static constexpr const char *metaTableName() { return "sn.GraphicsPipeline"; }

  enum FeatureFlag : uint32_t {
    DepthTest = 1 << 0,
    Instanced = 1 << 1,
    Animation = 1 << 2,
    Tangent = 1 << 3,
  };

  GraphicsPipeline() = default;

  void setVertexShader(const Shader &shader);
  void setVertexInstancedShader(const Shader &shader);
  void setFragmentShader(const Shader &shader);

  void setEnableDepthTest(bool enable);
  void setEnableInstanced(bool enable);
  void setEnableAnimation(bool enable);
  void setEnableTangent(bool enable);

  void build();

  const std::bitset<32> &getFeatureFlags() const { return featureFlags; }

  Ptr<gpu::GraphicsPipeline> get() const { return pipeline; }

private:
  Shader vertexShader;
  Shader fragmentShader;
  Ptr<gpu::GraphicsPipeline> pipeline;
  std::bitset<32> featureFlags;
};
} // namespace sinen

#endif // !SINEN_RENDER_PIPELINE_HPP
