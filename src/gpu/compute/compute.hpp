#ifndef SINEN_COMPUTE_HPP
#define SINEN_COMPUTE_HPP

#include "compute_buffer.hpp"
#include "compute_pipeline.hpp"

#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>

namespace sinen {
namespace gpu {
class AccelerationStructure;
}

class Compute {
public:
  static void shutdown();
  static void setComputePipeline(const ComputePipeline &pipeline);
  static void resetComputePipeline();
  static void setUniformBuffer(UInt32 slotIndex, const Buffer &buffer);
  static void setStorageBuffer(UInt32 slotIndex, const ComputeBuffer &buffer);
  static void resetStorageBuffer(UInt32 slotIndex);
  static void resetAllStorageBuffers();
  static void setAccelerationStructure(
      UInt32 slotIndex,
      const Ptr<gpu::AccelerationStructure> &accelerationStructure);
  static void resetAccelerationStructure(UInt32 slotIndex);
  static void resetAllAccelerationStructures();
  static void dispatch(UInt32 groupCountX, UInt32 groupCountY = 1,
                       UInt32 groupCountZ = 1);
};
} // namespace sinen

#endif
