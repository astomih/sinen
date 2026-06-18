#ifndef SINEN_COMPUTE_PIPELINE_HPP
#define SINEN_COMPUTE_PIPELINE_HPP

#include <gpu/gpu_compute_pipeline.hpp>
#include <gpu/shader/shader.hpp>

namespace sinen {
class ComputePipeline {
public:
  static constexpr const char *metaTableName() { return "sn.ComputePipeline"; }

  ComputePipeline() = default;
  ~ComputePipeline() = default;

  void setShader(const Shader &shader);
  void setThreadGroupSize(UInt32 x, UInt32 y = 1, UInt32 z = 1);
  void build();
  bool findUniformBufferSlot(StringView name, UInt32 &slot) const;
  Ptr<gpu::ComputePipeline> get() const { return pipeline; }

private:
  Shader computeShader;
  UInt32 threadCountX = 1;
  UInt32 threadCountY = 1;
  UInt32 threadCountZ = 1;
  Ptr<gpu::ComputePipeline> pipeline;
};
} // namespace sinen

#endif
