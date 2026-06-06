#ifndef SINEN_RAYTRACING_HPP
#define SINEN_RAYTRACING_HPP

#include "gpu/gpu_ray_tracing.hpp"
#include "gpu/shader/shader.hpp"

#include <core/buffer/buffer.hpp>
#include <core/data/string.hpp>

namespace sinen {
class RaytracingAccelerationStructure {
public:
  static constexpr const char *metaTableName() {
    return "sn.Raytracing.AccelerationStructure";
  }

  explicit RaytracingAccelerationStructure(
      Ptr<gpu::AccelerationStructure> accelerationStructure = nullptr);

  Ptr<gpu::AccelerationStructure> getRaw() const;
  UInt64 getDeviceAddress() const;

private:
  Ptr<gpu::AccelerationStructure> accelerationStructure;
};

class RaytracingPipeline {
public:
  static constexpr const char *metaTableName() {
    return "sn.Raytracing.Pipeline";
  }

  void addShader(const Shader &shader, const char *exportName);
  void addHitGroup(const char *exportName, UInt32 closestHitShaderIndex,
                   UInt32 anyHitShaderIndex, UInt32 intersectionShaderIndex);
  void setMaxPayloadSize(UInt32 value);
  void setMaxAttributeSize(UInt32 value);
  void setMaxRecursionDepth(UInt32 value);
  UInt32 getShaderGroupHandleSize() const;
  Ptr<gpu::RayTracingPipeline> getRaw() const;
  bool isReady() const;

  void build();
  Buffer getShaderGroupHandles(UInt32 firstGroup, UInt32 groupCount) const;

private:
  struct ShaderEntry {
    Shader shader;
    String exportName;
  };
  struct HitGroupEntry {
    String exportName;
    UInt32 closestHitShaderIndex = UINT32_MAX;
    UInt32 anyHitShaderIndex = UINT32_MAX;
    UInt32 intersectionShaderIndex = UINT32_MAX;
  };

  Array<ShaderEntry> shaders;
  Array<HitGroupEntry> hitGroups;
  Ptr<gpu::RayTracingPipeline> pipeline;
  UInt32 maxPayloadSize = 32;
  UInt32 maxAttributeSize = 8;
  UInt32 maxRecursionDepth = 1;
};

struct RaytracingDispatchInfo {
  gpu::RayTracingShaderTableRegion rayGeneration;
  gpu::RayTracingShaderTableRegion miss;
  gpu::RayTracingShaderTableRegion hit;
  gpu::RayTracingShaderTableRegion callable;
  Array<Buffer> uniforms;
  Array<Ptr<gpu::AccelerationStructure>> accelerationStructures;
  Array<gpu::StorageBufferBinding> storageBuffers;
  UInt32 width = 1;
  UInt32 height = 1;
  UInt32 depth = 1;
};

Ptr<RaytracingAccelerationStructure> createBottomLevelAccelerationStructure(
    const Array<gpu::RayTracingGeometry> &geometries,
    gpu::RayTracingBuildFlags flags, String *error = nullptr);
Ptr<RaytracingAccelerationStructure> createTopLevelAccelerationStructure(
    const Array<gpu::RayTracingInstance> &instances,
    gpu::RayTracingBuildFlags flags, String *error = nullptr);
bool dispatchRays(const RaytracingPipeline &pipeline,
                  const RaytracingDispatchInfo &info, String *error = nullptr);
} // namespace sinen

#endif
