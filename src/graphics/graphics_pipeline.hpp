#ifndef SINEN_RENDER_PIPELINE_HPP
#define SINEN_RENDER_PIPELINE_HPP
#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/shader/shader.hpp>

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
    DepthWrite = 1 << 4,
  };

  GraphicsPipeline() = default;
  ~GraphicsPipeline();

  void setVertexShader(const Shader &shader);
  void setVertexInstancedShader(const Shader &shader);
  void setFragmentShader(const Shader &shader);

  void setEnableDepthTest(bool enable);
  void setEnableDepthWrite(bool enable);
  void setEnableInstanced(bool enable);
  void setEnableAnimation(bool enable);
  void setEnableTangent(bool enable);

  void build();

  const std::bitset<32> &getFeatureFlags() const { return featureFlags; }
  bool findUniformBufferSlot(StringView name, UInt32 &slot) const;
  bool findTextureSlot(StringView name, UInt32 &slot) const;

  Ptr<gpu::GraphicsPipeline> get() const { return pipeline; }

private:
  Shader vertexShader;
  Shader fragmentShader;
  Ptr<gpu::GraphicsPipeline> pipeline;
  std::bitset<32> featureFlags;
};
} // namespace sinen

#endif // !SINEN_RENDER_PIPELINE_HPP
