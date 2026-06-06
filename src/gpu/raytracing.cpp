#include "raytracing.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>

#include <cstring>
#include <functional>
#include <memory>

namespace sinen {
namespace {
void setError(String *error, const char *message) {
  if (error) {
    *error = message;
  }
}

Ptr<gpu::Buffer> createStorageBuffer(UInt32 size) {
  if (size == 0) {
    return nullptr;
  }
  gpu::Buffer::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.usage = gpu::BufferUsage::Storage;
  info.size = size;
  return Graphics::getDevice()->createBuffer(info);
}

Ptr<gpu::Buffer> uploadStorageBuffer(const Buffer &data) {
  if (data.size() == 0) {
    return nullptr;
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  auto buffer = createStorageBuffer(static_cast<UInt32>(data.size()));

  gpu::TransferBuffer::CreateInfo transferInfo{};
  transferInfo.allocator = allocator;
  transferInfo.size = static_cast<UInt32>(data.size());
  transferInfo.usage = gpu::TransferBufferUsage::Upload;
  auto transfer = device->createTransferBuffer(transferInfo);
  auto *mapped = transfer->map(true);
  if (mapped) {
    std::memcpy(mapped, data.data(), data.size());
  }
  transfer->unmap();

  auto commandBuffer = device->acquireCommandBuffer({allocator});
  auto copyPass = commandBuffer->beginCopyPass();
  gpu::BufferTransferInfo src{};
  src.transferBuffer = transfer;
  src.offset = 0;
  gpu::BufferRegion dst{};
  dst.buffer = buffer;
  dst.offset = 0;
  dst.size = static_cast<UInt32>(data.size());
  copyPass->uploadBuffer(src, dst, true);
  commandBuffer->endCopyPass(copyPass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();

  return buffer;
}
} // namespace

RaytracingAccelerationStructure::RaytracingAccelerationStructure(
    Ptr<gpu::AccelerationStructure> accelerationStructure)
    : accelerationStructure(std::move(accelerationStructure)) {}

Ptr<gpu::AccelerationStructure> RaytracingAccelerationStructure::getRaw() const {
  return accelerationStructure;
}

UInt64 RaytracingAccelerationStructure::getDeviceAddress() const {
  return accelerationStructure ? accelerationStructure->getDeviceAddress() : 0;
}

void RaytracingPipeline::addShader(const Shader &shader,
                                   const char *exportName) {
  shaders.push_back({shader, exportName ? String(exportName) : String()});
}

void RaytracingPipeline::addHitGroup(const char *exportName,
                                     UInt32 closestHitShaderIndex,
                                     UInt32 anyHitShaderIndex,
                                     UInt32 intersectionShaderIndex) {
  hitGroups.push_back({exportName ? String(exportName) : String(),
                       closestHitShaderIndex, anyHitShaderIndex,
                       intersectionShaderIndex});
}

void RaytracingPipeline::setMaxPayloadSize(UInt32 value) {
  maxPayloadSize = value;
}

void RaytracingPipeline::setMaxAttributeSize(UInt32 value) {
  maxAttributeSize = value;
}

void RaytracingPipeline::setMaxRecursionDepth(UInt32 value) {
  maxRecursionDepth = value;
}

UInt32 RaytracingPipeline::getShaderGroupHandleSize() const {
  return pipeline ? pipeline->getShaderGroupHandleSize() : 0;
}

Ptr<gpu::RayTracingPipeline> RaytracingPipeline::getRaw() const {
  return pipeline;
}

bool RaytracingPipeline::isReady() const { return pipeline != nullptr; }

void RaytracingPipeline::build() {
  const TaskGroup group = LoadContext::current();
  group.add();
  const bool inSetup = (LoadContext::currentPtr() != nullptr);

  auto buildOrRetry = std::make_shared<std::function<void()>>();
  *buildOrRetry = [this, buildOrRetry, group]() {
    if (pipeline) {
      group.done();
      return;
    }

    for (auto &entry : shaders) {
      if (!entry.shader.getRaw()) {
        Graphics::addPreDrawFunc(*buildOrRetry);
        return;
      }
    }

    auto allocator = GlobalAllocator::get();
    gpu::RayTracingPipeline::CreateInfo info(allocator);
    info.maxPayloadSize = maxPayloadSize;
    info.maxAttributeSize = maxAttributeSize;
    info.maxRecursionDepth = maxRecursionDepth;
    for (auto &entry : shaders) {
      gpu::RayTracingShader shaderInfo{};
      shaderInfo.shader = entry.shader.getRaw();
      shaderInfo.exportName =
          entry.exportName.empty() ? nullptr : entry.exportName.c_str();
      info.shaders.push_back(shaderInfo);
    }
    for (auto &entry : hitGroups) {
      gpu::RayTracingHitGroup hitGroup{};
      hitGroup.exportName =
          entry.exportName.empty() ? nullptr : entry.exportName.c_str();
      hitGroup.closestHitShaderIndex = entry.closestHitShaderIndex;
      hitGroup.anyHitShaderIndex = entry.anyHitShaderIndex;
      hitGroup.intersectionShaderIndex = entry.intersectionShaderIndex;
      info.hitGroups.push_back(hitGroup);
    }
    pipeline = Graphics::getDevice()->createRayTracingPipeline(info);
    group.done();
  };

  if (!inSetup) {
    (*buildOrRetry)();
    return;
  }
  Graphics::addPreDrawFunc(*buildOrRetry);
}

Buffer RaytracingPipeline::getShaderGroupHandles(UInt32 firstGroup,
                                                 UInt32 groupCount) const {
  if (!pipeline || groupCount == 0) {
    return Buffer(BufferType::Binary, Ptr<void>(), 0);
  }
  const UInt32 handleSize = pipeline->getShaderGroupHandleSize();
  Buffer out =
      makeBuffer(static_cast<size_t>(handleSize) * groupCount,
                 BufferType::Binary);
  if (!pipeline->getShaderGroupHandles(firstGroup, groupCount, out.data(),
                                       out.size())) {
    return Buffer(BufferType::Binary, Ptr<void>(), 0);
  }
  return out;
}

Ptr<RaytracingAccelerationStructure> createBottomLevelAccelerationStructure(
    const Array<gpu::RayTracingGeometry> &geometries,
    gpu::RayTracingBuildFlags flags, String *error) {
  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  const auto sizes =
      device->getBottomLevelAccelerationStructureBuildSizes(geometries, flags);

  gpu::AccelerationStructure::CreateInfo asInfo{};
  asInfo.allocator = allocator;
  asInfo.type = gpu::AccelerationStructure::Type::BottomLevel;
  asInfo.size = sizes.accelerationStructureSize;
  auto accelerationStructure = device->createAccelerationStructure(asInfo);
  if (!accelerationStructure) {
    setError(error, "Raytracing.createBottomLevel failed to create "
                    "acceleration structure");
    return nullptr;
  }

  auto scratch =
      createStorageBuffer(static_cast<UInt32>(sizes.buildScratchSize));
  auto commandBuffer = device->acquireCommandBuffer({allocator});
  if (!commandBuffer) {
    setError(error, "Raytracing.createBottomLevel failed to acquire command "
                    "buffer");
    return nullptr;
  }
  auto pass = commandBuffer->beginRayTracingPass();
  if (!pass) {
    setError(error,
             "Raytracing.createBottomLevel failed to begin ray tracing pass");
    return nullptr;
  }
  pass->buildBottomLevelAccelerationStructure(accelerationStructure, geometries,
                                              scratch, 0, flags);
  commandBuffer->endRayTracingPass(pass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();

  return makePtr<RaytracingAccelerationStructure>(accelerationStructure);
}

Ptr<RaytracingAccelerationStructure> createTopLevelAccelerationStructure(
    const Array<gpu::RayTracingInstance> &instances,
    gpu::RayTracingBuildFlags flags, String *error) {
  Buffer instanceData =
      makeBuffer(sizeof(gpu::RayTracingInstance) * instances.size(),
                 BufferType::Binary);
  if (!instances.empty()) {
    std::memcpy(instanceData.data(), instances.data(), instanceData.size());
  }
  auto instanceBuffer = uploadStorageBuffer(instanceData);

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  const auto sizes = device->getTopLevelAccelerationStructureBuildSizes(
      static_cast<UInt32>(instances.size()), flags);

  gpu::AccelerationStructure::CreateInfo asInfo{};
  asInfo.allocator = allocator;
  asInfo.type = gpu::AccelerationStructure::Type::TopLevel;
  asInfo.size = sizes.accelerationStructureSize;
  auto accelerationStructure = device->createAccelerationStructure(asInfo);
  if (!accelerationStructure) {
    setError(error, "Raytracing.createTopLevel failed to create acceleration "
                    "structure");
    return nullptr;
  }

  auto scratch =
      createStorageBuffer(static_cast<UInt32>(sizes.buildScratchSize));
  auto commandBuffer = device->acquireCommandBuffer({allocator});
  if (!commandBuffer) {
    setError(error,
             "Raytracing.createTopLevel failed to acquire command buffer");
    return nullptr;
  }
  auto pass = commandBuffer->beginRayTracingPass();
  if (!pass) {
    setError(error,
             "Raytracing.createTopLevel failed to begin ray tracing pass");
    return nullptr;
  }
  pass->buildTopLevelAccelerationStructure(
      accelerationStructure, gpu::BufferBinding{instanceBuffer, 0},
      static_cast<UInt32>(instances.size()), scratch, 0, flags);
  commandBuffer->endRayTracingPass(pass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();

  return makePtr<RaytracingAccelerationStructure>(accelerationStructure);
}

bool dispatchRays(const RaytracingPipeline &pipeline,
                  const RaytracingDispatchInfo &info, String *error) {
  if (!pipeline.getRaw()) {
    setError(error, "Raytracing.dispatch called with an unbuilt pipeline");
    return false;
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  auto commandBuffer = device->acquireCommandBuffer({allocator});
  if (!commandBuffer) {
    setError(error, "Raytracing.dispatch failed to acquire command buffer");
    return false;
  }

  for (size_t i = 0; i < info.uniforms.size(); ++i) {
    const auto &buffer = info.uniforms[i];
    commandBuffer->pushComputeUniformData(static_cast<UInt32>(i),
                                          buffer.data(), buffer.size());
  }

  auto pass = commandBuffer->beginRayTracingPass();
  if (!pass) {
    setError(error, "Raytracing.dispatch failed to begin ray tracing pass");
    return false;
  }
  pass->bindRayTracingPipeline(pipeline.getRaw());
  pass->bindAccelerationStructures(0, info.accelerationStructures);
  pass->bindStorageBuffers(0, info.storageBuffers);
  pass->dispatchRays(info.rayGeneration, info.miss, info.hit, info.callable,
                     info.width, info.height, info.depth);
  commandBuffer->endRayTracingPass(pass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();
  return true;
}
} // namespace sinen
