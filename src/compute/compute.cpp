#include "compute.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/logger/log.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

#include <algorithm>
#include <optional>

namespace sinen {
static std::optional<ComputePipeline> currentPipeline;
static Hashmap<UInt32, Buffer> currentUniformBindings;
static Hashmap<UInt32, ComputeBuffer> currentStorageBufferBindings;
static Hashmap<UInt32, Ptr<gpu::AccelerationStructure>>
    currentAccelerationStructureBindings;

void Compute::setComputePipeline(const ComputePipeline &pipeline) {
  currentPipeline = pipeline;
}

void Compute::resetComputePipeline() { currentPipeline = std::nullopt; }

void Compute::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  currentUniformBindings.insert_or_assign(slotIndex, buffer);
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

static int lComputeSetComputePipeline(lua_State *L) {
  auto &pipeline = udPtr<ComputePipeline>(L, 1);
  Compute::setComputePipeline(*pipeline);
  return 0;
}

static int lComputeResetComputePipeline(lua_State *L) {
  (void)L;
  Compute::resetComputePipeline();
  return 0;
}

static int lComputeSetUniformBuffer(lua_State *L) {
  auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &buffer = udValue<Buffer>(L, 2);
  Compute::setUniformBuffer(slot, buffer);
  return 0;
}

static int lComputeSetStorageBuffer(lua_State *L) {
  auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &buffer = udPtr<ComputeBuffer>(L, 2);
  Compute::setStorageBuffer(slot, *buffer);
  return 0;
}

static int lComputeResetStorageBuffer(lua_State *L) {
  auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Compute::resetStorageBuffer(slot);
  return 0;
}

static int lComputeResetAllStorageBuffers(lua_State *L) {
  (void)L;
  Compute::resetAllStorageBuffers();
  return 0;
}

static int lComputeDispatch(lua_State *L) {
  auto x = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto y = static_cast<UInt32>(luaL_optinteger(L, 2, 1));
  auto z = static_cast<UInt32>(luaL_optinteger(L, 3, 1));
  Compute::dispatch(x, y, z);
  return 0;
}

void registerCompute(lua_State *L) {
  pushSnNamed(L, "Compute");
  luaPushcfunction2(L, lComputeSetComputePipeline);
  lua_setfield(L, -2, "setComputePipeline");
  luaPushcfunction2(L, lComputeResetComputePipeline);
  lua_setfield(L, -2, "resetComputePipeline");
  luaPushcfunction2(L, lComputeSetUniformBuffer);
  lua_setfield(L, -2, "setUniformBuffer");
  luaPushcfunction2(L, lComputeSetStorageBuffer);
  lua_setfield(L, -2, "setStorageBuffer");
  luaPushcfunction2(L, lComputeResetStorageBuffer);
  lua_setfield(L, -2, "resetStorageBuffer");
  luaPushcfunction2(L, lComputeResetAllStorageBuffers);
  lua_setfield(L, -2, "resetAllStorageBuffers");
  luaPushcfunction2(L, lComputeDispatch);
  lua_setfield(L, -2, "dispatch");
  lua_pop(L, 1);
}
} // namespace sinen
