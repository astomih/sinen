#ifndef SINEN_GPU_COMPUTE_PIPELINE_HPP
#define SINEN_GPU_COMPUTE_PIPELINE_HPP
#include "gpu_shader.hpp"
namespace sinen::gpu {
class ComputePipeline {
public:
  struct CreateInfo {
    Allocator *allocator;
    Ptr<Shader> computeShader;
    UInt32 threadCountX = 1;
    UInt32 threadCountY = 1;
    UInt32 threadCountZ = 1;
  };
  virtual ~ComputePipeline() = default;
  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  ComputePipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
