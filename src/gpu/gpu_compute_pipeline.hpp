#ifndef SINEN_GPU_COMPUTE_PIPELINE_HPP
#define SINEN_GPU_COMPUTE_PIPELINE_HPP
#include "gpu_shader.hpp"
namespace sinen::gpu {
class ComputePipeline {
public:
  struct CreateInfo {
    Allocator *allocator;
    Ptr<Shader> computeShader;
  };
  virtual ~ComputePipeline() = default;

protected:
  ComputePipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
