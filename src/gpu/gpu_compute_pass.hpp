#ifndef SINEN_GPU_COMPUTE_PASS_HPP
#define SINEN_GPU_COMPUTE_PASS_HPP
#include "gpu_buffer.hpp"
#include "gpu_compute_pipeline.hpp"
#include "gpu_texture.hpp"

namespace sinen::gpu {
struct StorageBufferBinding {
  Ptr<Buffer> buffer;
  bool cycle = false;
};

struct StorageTextureBinding {
  Ptr<Texture> texture;
  UInt32 mipLevel = 0;
  UInt32 layer = 0;
  bool cycle = false;
};

class ComputePass {
public:
  virtual ~ComputePass() = default;

  virtual void bindComputePipeline(Ptr<ComputePipeline> computePipeline) = 0;
  virtual void dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                                  UInt32 groupCountZ) = 0;

protected:
  ComputePass() = default;
};
} // namespace sinen::gpu
#endif
