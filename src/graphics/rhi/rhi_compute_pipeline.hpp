#ifndef SINEN_RHI_COMPUTE_PIPELINE_HPP
#define SINEN_RHI_COMPUTE_PIPELINE_HPP
#include "rhi_shader.hpp"
namespace sinen::rhi {
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
} // namespace sinen::rhi
#endif