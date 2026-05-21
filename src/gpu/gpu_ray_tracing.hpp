#ifndef SINEN_GPU_RAY_TRACING_HPP
#define SINEN_GPU_RAY_TRACING_HPP

#include "gpu_buffer.hpp"
#include "gpu_compute_pass.hpp"
#include "gpu_graphics_pipeline.hpp"
#include "gpu_render_pass.hpp"
#include "gpu_shader.hpp"

#include <core/data/array.hpp>

#include <cstdint>

namespace sinen::gpu {
class AccelerationStructure {
public:
  enum class Type { BottomLevel, TopLevel };

  struct CreateInfo {
    Allocator *allocator = nullptr;
    Type type = Type::BottomLevel;
    UInt64 size = 0;
  };

  virtual ~AccelerationStructure() = default;
  const CreateInfo &getCreateInfo() const { return createInfo; }
  virtual UInt64 getDeviceAddress() const { return 0; }

protected:
  explicit AccelerationStructure(const CreateInfo &createInfo)
      : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

struct RayTracingAccelerationStructureBuildSizes {
  UInt64 accelerationStructureSize = 0;
  UInt64 buildScratchSize = 0;
  UInt64 updateScratchSize = 0;
};

enum class RayTracingBuildFlags : UInt32 {
  None = 0,
  AllowUpdate = 1u << 0,
  PreferFastTrace = 1u << 1,
  PreferFastBuild = 1u << 2,
  MinimizeMemory = 1u << 3,
};

inline RayTracingBuildFlags operator|(RayTracingBuildFlags a,
                                      RayTracingBuildFlags b) {
  return static_cast<RayTracingBuildFlags>(static_cast<UInt32>(a) |
                                           static_cast<UInt32>(b));
}

inline bool hasFlag(RayTracingBuildFlags value, RayTracingBuildFlags flag) {
  return (static_cast<UInt32>(value) & static_cast<UInt32>(flag)) != 0;
}

enum class RayTracingGeometryType { Triangles, Aabbs };

struct RayTracingTriangleGeometry {
  Ptr<Buffer> vertexBuffer;
  UInt64 vertexOffset = 0;
  UInt32 vertexStride = 0;
  UInt32 vertexCount = 0;
  VertexElementFormat vertexFormat = VertexElementFormat::Float3;
  Ptr<Buffer> indexBuffer;
  UInt64 indexOffset = 0;
  UInt32 indexCount = 0;
  IndexElementSize indexElementSize = IndexElementSize::Uint32;
  bool opaque = true;
};

struct RayTracingAabbGeometry {
  Ptr<Buffer> buffer;
  UInt64 offset = 0;
  UInt32 stride = 24;
  UInt32 count = 0;
  bool opaque = true;
};

struct RayTracingGeometry {
  RayTracingGeometryType type = RayTracingGeometryType::Triangles;
  RayTracingTriangleGeometry triangles;
  RayTracingAabbGeometry aabbs;
};

enum class RayTracingInstanceFlags : UInt8 {
  None = 0,
  TriangleCullDisable = 1u << 0,
  TriangleFrontCounterClockwise = 1u << 1,
  ForceOpaque = 1u << 2,
  ForceNoOpaque = 1u << 3,
};

struct RayTracingInstance {
  float transform[3][4]{};
  UInt32 instanceCustomIndex : 24 = 0;
  UInt32 mask : 8 = 0xff;
  UInt32 shaderBindingTableRecordOffset : 24 = 0;
  UInt32 flags : 8 = 0;
  UInt64 accelerationStructureAddress = 0;
};

struct RayTracingShader {
  Ptr<Shader> shader;
  const char *exportName = nullptr;
};

struct RayTracingHitGroup {
  const char *exportName = nullptr;
  UInt32 closestHitShaderIndex = UINT32_MAX;
  UInt32 anyHitShaderIndex = UINT32_MAX;
  UInt32 intersectionShaderIndex = UINT32_MAX;
};

class RayTracingPipeline {
public:
  struct CreateInfo {
    explicit CreateInfo(Allocator *allocator)
        : allocator(allocator), shaders(allocator), hitGroups(allocator) {}

    Allocator *allocator = nullptr;
    Array<RayTracingShader> shaders;
    Array<RayTracingHitGroup> hitGroups;
    UInt32 maxPayloadSize = 32;
    UInt32 maxAttributeSize = 8;
    UInt32 maxRecursionDepth = 1;
  };

  virtual ~RayTracingPipeline() = default;
  const CreateInfo &getCreateInfo() const { return createInfo; }
  virtual UInt32 getShaderGroupHandleSize() const { return 0; }
  virtual bool getShaderGroupHandles(UInt32, UInt32, void *, Size) const {
    return false;
  }

protected:
  explicit RayTracingPipeline(const CreateInfo &createInfo)
      : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

struct RayTracingShaderTableRegion {
  Ptr<Buffer> buffer;
  UInt64 offset = 0;
  UInt64 size = 0;
  UInt64 stride = 0;
};

class RayTracingPass {
public:
  virtual ~RayTracingPass() = default;

  virtual void buildBottomLevelAccelerationStructure(
      Ptr<AccelerationStructure> dst,
      const Array<RayTracingGeometry> &geometries, Ptr<Buffer> scratchBuffer,
      UInt64 scratchOffset, RayTracingBuildFlags flags,
      Ptr<AccelerationStructure> src = nullptr) = 0;
  virtual void buildTopLevelAccelerationStructure(
      Ptr<AccelerationStructure> dst, const BufferBinding &instances,
      UInt32 instanceCount, Ptr<Buffer> scratchBuffer, UInt64 scratchOffset,
      RayTracingBuildFlags flags, Ptr<AccelerationStructure> src = nullptr) = 0;
  virtual void bindAccelerationStructures(
      UInt32 startSlot,
      const Array<Ptr<AccelerationStructure>> &accelerationStructures) = 0;
  virtual void
  bindStorageBuffers(UInt32 startSlot,
                     const Array<StorageBufferBinding> &storageBuffers) = 0;
  virtual void bindRayTracingPipeline(Ptr<RayTracingPipeline> pipeline) = 0;
  virtual void dispatchRays(const RayTracingShaderTableRegion &rayGeneration,
                            const RayTracingShaderTableRegion &miss,
                            const RayTracingShaderTableRegion &hit,
                            const RayTracingShaderTableRegion &callable,
                            UInt32 width, UInt32 height, UInt32 depth) = 0;

protected:
  RayTracingPass() = default;
};
} // namespace sinen::gpu

#endif
