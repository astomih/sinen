#include <gpu/compute/compute.hpp>

#include <core/allocator/global_allocator.hpp>
#include <core/logger/log.hpp>
#include <graphics/graphics.hpp>

#include <algorithm>
#include <optional>

namespace sinen {
static std::optional<ComputePipeline> currentPipeline;
static Hashmap<UInt32, Buffer> currentUniformBindings;
static Hashmap<UInt32, ComputeBuffer> currentStorageBufferBindings;
static Hashmap<UInt32, Ptr<gpu::AccelerationStructure>>
    currentAccelerationStructureBindings;

void Compute::shutdown() {
  currentPipeline = std::nullopt;
  currentUniformBindings.clear();
  currentStorageBufferBindings.clear();
  currentAccelerationStructureBindings.clear();
}

void Compute::setComputePipeline(const ComputePipeline &pipeline) {
  currentPipeline = pipeline;
}

void Compute::resetComputePipeline() { currentPipeline = std::nullopt; }

void Compute::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  currentUniformBindings.insert_or_assign(slotIndex, buffer);
}

void Compute::setUniformBuffer(StringView name, const Buffer &buffer) {
  if (!currentPipeline.has_value() || currentPipeline->get() == nullptr) {
    Log::error("Compute::setUniformBuffer('%.*s') called without a ready "
               "compute pipeline",
               static_cast<int>(name.size()), name.data());
    return;
  }

  UInt32 slot = 0;
  if (!currentPipeline->findUniformBufferSlot(name, slot)) {
    Log::error("Uniform buffer '%.*s' was not found in the current compute "
               "pipeline",
               static_cast<int>(name.size()), name.data());
    return;
  }
  setUniformBuffer(slot, buffer);
}

void Compute::setStorageBuffer(UInt32 slotIndex, const ComputeBuffer &buffer) {
  currentStorageBufferBindings.insert_or_assign(slotIndex, buffer);
}

void Compute::resetStorageBuffer(UInt32 slotIndex) {
  currentStorageBufferBindings.erase(slotIndex);
}

void Compute::resetAllStorageBuffers() { currentStorageBufferBindings.clear(); }

void Compute::setAccelerationStructure(
    UInt32 slotIndex,
    const Ptr<gpu::AccelerationStructure> &accelerationStructure) {
  currentAccelerationStructureBindings.insert_or_assign(slotIndex,
                                                        accelerationStructure);
}

void Compute::resetAccelerationStructure(UInt32 slotIndex) {
  currentAccelerationStructureBindings.erase(slotIndex);
}

void Compute::resetAllAccelerationStructures() {
  currentAccelerationStructureBindings.clear();
}

void Compute::dispatch(UInt32 groupCountX, UInt32 groupCountY,
                       UInt32 groupCountZ) {
  if (!currentPipeline.has_value() || currentPipeline->get() == nullptr) {
    Log::error("Compute::dispatch called without a ready compute pipeline");
    return;
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  auto commandBuffer = device->acquireCommandBuffer({allocator});

  for (auto &binding : currentUniformBindings) {
    commandBuffer->pushComputeUniformData(binding.first, binding.second.data(),
                                          binding.second.size());
  }

  UInt32 maxStorageSlot = 0;
  for (const auto &binding : currentStorageBufferBindings) {
    maxStorageSlot = std::max(maxStorageSlot, binding.first);
  }

  Array<gpu::StorageTextureBinding> storageTextures(allocator);
  Array<gpu::StorageBufferBinding> storageBuffers(allocator);
  if (!currentStorageBufferBindings.empty()) {
    storageBuffers.resize(maxStorageSlot + 1);
    for (auto &binding : currentStorageBufferBindings) {
      storageBuffers[binding.first].buffer = binding.second.getRaw();
      storageBuffers[binding.first].cycle = false;
    }
  }

  UInt32 maxAccelerationStructureSlot = 0;
  for (const auto &binding : currentAccelerationStructureBindings) {
    maxAccelerationStructureSlot =
        std::max(maxAccelerationStructureSlot, binding.first);
  }
  Array<Ptr<gpu::AccelerationStructure>> accelerationStructures(allocator);
  if (!currentAccelerationStructureBindings.empty()) {
    accelerationStructures.resize(maxAccelerationStructureSlot + 1);
    for (auto &binding : currentAccelerationStructureBindings) {
      accelerationStructures[binding.first] = binding.second;
    }
  }

  auto computePass =
      commandBuffer->beginComputePass(storageTextures, storageBuffers);
  computePass->bindComputePipeline(currentPipeline->get());
  computePass->bindAccelerationStructures(0, accelerationStructures);
  computePass->dispatchWorkgroups(groupCountX, groupCountY, groupCountZ);
  commandBuffer->endComputePass(computePass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();

  currentUniformBindings.clear();
  currentStorageBufferBindings.clear();
  currentAccelerationStructureBindings.clear();
}

} // namespace sinen
