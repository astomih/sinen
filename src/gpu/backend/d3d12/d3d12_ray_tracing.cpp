#include "d3d12_device.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_command.hpp"
#include "d3d12_convert.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <vector>

namespace sinen::gpu::d3d12 {
namespace {
void logIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s (HRESULT 0x%08X)", message,
                 static_cast<unsigned int>(hr));
  }
}

D3D12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE type) {
  D3D12_HEAP_PROPERTIES props{};
  props.Type = type;
  props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  props.CreationNodeMask = 1;
  props.VisibleNodeMask = 1;
  return props;
}

D3D12_RESOURCE_DESC bufferDesc(UINT64 size) {
  D3D12_RESOURCE_DESC desc{};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  return desc;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS
rayTracingBuildFlagsFrom(gpu::RayTracingBuildFlags flags) {
  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS out =
      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::AllowUpdate)) {
    out |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::PreferFastTrace)) {
    out |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::PreferFastBuild)) {
    out |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::MinimizeMemory)) {
    out |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
  }
  return out;
}

std::wstring wideName(const char *name) {
  if (!name || !*name) {
    name = "main";
  }
  std::wstring out;
  while (*name) {
    out.push_back(static_cast<wchar_t>(*name++));
  }
  return out;
}

D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags(bool opaque) {
  return opaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE
                : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
}

D3D12_RAYTRACING_GEOMETRY_DESC
geometryDescFrom(const gpu::RayTracingGeometry &geometry) {
  D3D12_RAYTRACING_GEOMETRY_DESC desc{};
  desc.Flags = geometryFlags(geometry.type == gpu::RayTracingGeometryType::Aabbs
                                 ? geometry.aabbs.opaque
                                 : geometry.triangles.opaque);
  if (geometry.type == gpu::RayTracingGeometryType::Aabbs) {
    auto buffer = downCast<Buffer>(geometry.aabbs.buffer);
    desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
    desc.AABBs.AABBCount = geometry.aabbs.count;
    desc.AABBs.AABBs.StartAddress =
        buffer ? buffer->getDeviceAddress() + geometry.aabbs.offset : 0;
    desc.AABBs.AABBs.StrideInBytes = geometry.aabbs.stride;
    return desc;
  }

  auto vertexBuffer = downCast<Buffer>(geometry.triangles.vertexBuffer);
  auto indexBuffer = downCast<Buffer>(geometry.triangles.indexBuffer);
  desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
  desc.Triangles.VertexBuffer.StartAddress =
      vertexBuffer
          ? vertexBuffer->getDeviceAddress() + geometry.triangles.vertexOffset
          : 0;
  desc.Triangles.VertexBuffer.StrideInBytes = geometry.triangles.vertexStride;
  desc.Triangles.VertexCount = geometry.triangles.vertexCount;
  desc.Triangles.VertexFormat =
      convert::vertexFormatFrom(geometry.triangles.vertexFormat);
  desc.Triangles.IndexBuffer = indexBuffer ? indexBuffer->getDeviceAddress() +
                                                 geometry.triangles.indexOffset
                                           : 0;
  desc.Triangles.IndexCount = geometry.triangles.indexCount;
  desc.Triangles.IndexFormat =
      indexBuffer
          ? convert::indexFormatFrom(geometry.triangles.indexElementSize)
          : DXGI_FORMAT_UNKNOWN;
  return desc;
}

D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS
bottomLevelInputs(const Array<gpu::RayTracingGeometry> &geometries,
                  std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> &nativeGeometries,
                  gpu::RayTracingBuildFlags flags) {
  nativeGeometries.clear();
  nativeGeometries.reserve(geometries.size());
  for (const auto &geometry : geometries) {
    nativeGeometries.push_back(geometryDescFrom(geometry));
  }

  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
  inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
  inputs.Flags = rayTracingBuildFlagsFrom(flags);
  inputs.NumDescs = static_cast<UINT>(nativeGeometries.size());
  inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
  inputs.pGeometryDescs = nativeGeometries.data();
  return inputs;
}

D3D12_GPU_VIRTUAL_ADDRESS
regionAddress(const gpu::RayTracingShaderTableRegion &region) {
  auto buffer = downCast<Buffer>(region.buffer);
  return buffer ? buffer->getDeviceAddress() + region.offset : 0;
}

LPCWSTR exportNameAt(const std::vector<std::wstring> &names,
                     UInt32 shaderIndex) {
  return shaderIndex < names.size() ? names[shaderIndex].c_str() : nullptr;
}
} // namespace

gpu::RayTracingAccelerationStructureBuildSizes
Device::getBottomLevelAccelerationStructureBuildSizes(
    const Array<gpu::RayTracingGeometry> &geometries,
    gpu::RayTracingBuildFlags flags) {
  if (!rayTracingSupported || !device5) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: bottom-level acceleration structure size query "
                "ignored because ray tracing is disabled");
    return {};
  }
  std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> nativeGeometries;
  auto inputs = bottomLevelInputs(geometries, nativeGeometries, flags);
  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
  device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
  return {info.ResultDataMaxSizeInBytes, info.ScratchDataSizeInBytes,
          info.UpdateScratchDataSizeInBytes};
}

gpu::RayTracingAccelerationStructureBuildSizes
Device::getTopLevelAccelerationStructureBuildSizes(
    UInt32 instanceCount, gpu::RayTracingBuildFlags flags) {
  if (!rayTracingSupported || !device5) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: top-level acceleration structure size query ignored "
                "because ray tracing is disabled");
    return {};
  }
  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
  inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
  inputs.Flags = rayTracingBuildFlagsFrom(flags);
  inputs.NumDescs = instanceCount;
  inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
  device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
  return {info.ResultDataMaxSizeInBytes, info.ScratchDataSizeInBytes,
          info.UpdateScratchDataSizeInBytes};
}

Ptr<gpu::AccelerationStructure> Device::createAccelerationStructure(
    const gpu::AccelerationStructure::CreateInfo &createInfo) {
  if (!rayTracingSupported || !device5 || createInfo.size == 0) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: createAccelerationStructure ignored because ray "
                "tracing is disabled or the requested size is zero");
    return nullptr;
  }
  auto heap = heapProperties(D3D12_HEAP_TYPE_DEFAULT);
  auto desc = bufferDesc(createInfo.size);
  desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &desc,
      D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr,
      IID_PPV_ARGS(&resource));
  logIfFailed(hr, "D3D12: create acceleration structure failed");
  if (FAILED(hr)) {
    return nullptr;
  }
  return makePtr<AccelerationStructure>(createInfo.allocator, createInfo, get(),
                                        resource);
}

Ptr<gpu::RayTracingPipeline> Device::createRayTracingPipeline(
    const gpu::RayTracingPipeline::CreateInfo &createInfo) {
  if (!rayTracingSupported || !device5 || !rayTracingRootSignature) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: createRayTracingPipeline ignored because ray tracing "
                "is disabled");
    return nullptr;
  }

  std::vector<std::wstring> exportNames;
  exportNames.reserve(createInfo.shaders.size() + createInfo.hitGroups.size());
  for (const auto &shader : createInfo.shaders) {
    auto native = downCast<Shader>(shader.shader);
    const char *name = shader.exportName;
    if (!name && native) {
      name = native->getCreateInfo().entrypoint;
    }
    exportNames.push_back(wideName(name));
  }
  for (const auto &hitGroup : createInfo.hitGroups) {
    exportNames.push_back(wideName(hitGroup.exportName));
  }

  std::vector<D3D12_EXPORT_DESC> exports(createInfo.shaders.size());
  std::vector<D3D12_DXIL_LIBRARY_DESC> libraries(createInfo.shaders.size());
  std::vector<D3D12_HIT_GROUP_DESC> hitGroups(createInfo.hitGroups.size());
  std::vector<D3D12_STATE_SUBOBJECT> subobjects;
  subobjects.reserve(createInfo.shaders.size() + createInfo.hitGroups.size() +
                     3);

  for (size_t i = 0; i < createInfo.shaders.size(); ++i) {
    auto shader = downCast<Shader>(createInfo.shaders[i].shader);
    if (!shader) {
      continue;
    }
    exports[i].Name = exportNames[i].c_str();
    exports[i].Flags = D3D12_EXPORT_FLAG_NONE;
    libraries[i].DXILLibrary = shader->getBytecode();
    libraries[i].NumExports = 1;
    libraries[i].pExports = &exports[i];

    D3D12_STATE_SUBOBJECT subobject{};
    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobject.pDesc = &libraries[i];
    subobjects.push_back(subobject);
  }

  const size_t hitGroupNameBase = createInfo.shaders.size();
  for (size_t i = 0; i < createInfo.hitGroups.size(); ++i) {
    const auto &src = createInfo.hitGroups[i];
    auto &dst = hitGroups[i];
    dst.HitGroupExport = exportNames[hitGroupNameBase + i].c_str();
    dst.Type = src.intersectionShaderIndex == UINT32_MAX
                   ? D3D12_HIT_GROUP_TYPE_TRIANGLES
                   : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
    if (src.closestHitShaderIndex != UINT32_MAX) {
      dst.ClosestHitShaderImport =
          exportNameAt(exportNames, src.closestHitShaderIndex);
    }
    if (src.anyHitShaderIndex != UINT32_MAX) {
      dst.AnyHitShaderImport = exportNameAt(exportNames, src.anyHitShaderIndex);
    }
    if (src.intersectionShaderIndex != UINT32_MAX) {
      dst.IntersectionShaderImport =
          exportNameAt(exportNames, src.intersectionShaderIndex);
    }

    D3D12_STATE_SUBOBJECT subobject{};
    subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    subobject.pDesc = &dst;
    subobjects.push_back(subobject);
  }

  D3D12_RAYTRACING_SHADER_CONFIG shaderConfig{};
  shaderConfig.MaxPayloadSizeInBytes = createInfo.maxPayloadSize;
  shaderConfig.MaxAttributeSizeInBytes = createInfo.maxAttributeSize;
  D3D12_STATE_SUBOBJECT shaderConfigSubobject{};
  shaderConfigSubobject.Type =
      D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
  shaderConfigSubobject.pDesc = &shaderConfig;
  subobjects.push_back(shaderConfigSubobject);

  D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature{};
  globalRootSignature.pGlobalRootSignature = rayTracingRootSignature.Get();
  D3D12_STATE_SUBOBJECT rootSignatureSubobject{};
  rootSignatureSubobject.Type =
      D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
  rootSignatureSubobject.pDesc = &globalRootSignature;
  subobjects.push_back(rootSignatureSubobject);

  D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig{};
  pipelineConfig.MaxTraceRecursionDepth = createInfo.maxRecursionDepth;
  D3D12_STATE_SUBOBJECT pipelineConfigSubobject{};
  pipelineConfigSubobject.Type =
      D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
  pipelineConfigSubobject.pDesc = &pipelineConfig;
  subobjects.push_back(pipelineConfigSubobject);

  D3D12_STATE_OBJECT_DESC desc{};
  desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
  desc.NumSubobjects = static_cast<UINT>(subobjects.size());
  desc.pSubobjects = subobjects.data();

  ComPtr<ID3D12StateObject> stateObject;
  HRESULT hr = device5->CreateStateObject(&desc, IID_PPV_ARGS(&stateObject));
  logIfFailed(hr, "D3D12: CreateStateObject ray tracing pipeline failed");
  if (FAILED(hr)) {
    return nullptr;
  }

  std::vector<std::wstring> shaderGroupExportNames;
  shaderGroupExportNames.reserve(createInfo.shaders.size() +
                                 createInfo.hitGroups.size());
  for (size_t i = 0; i < createInfo.shaders.size(); ++i) {
    auto shader = downCast<Shader>(createInfo.shaders[i].shader);
    if (!shader) {
      continue;
    }
    const auto stage = shader->getCreateInfo().stage;
    if (stage == ShaderStage::AnyHit || stage == ShaderStage::ClosestHit ||
        stage == ShaderStage::Intersection) {
      continue;
    }
    shaderGroupExportNames.push_back(exportNames[i]);
  }
  for (size_t i = 0; i < createInfo.hitGroups.size(); ++i) {
    shaderGroupExportNames.push_back(exportNames[hitGroupNameBase + i]);
  }

  return makePtr<RayTracingPipeline>(createInfo.allocator, createInfo, get(),
                                     rayTracingRootSignature, stateObject,
                                     std::move(shaderGroupExportNames));
}

void RayTracingPass::buildBottomLevelAccelerationStructure(
    Ptr<gpu::AccelerationStructure> dst,
    const Array<gpu::RayTracingGeometry> &geometries,
    Ptr<gpu::Buffer> scratchBuffer, UInt64 scratchOffset,
    gpu::RayTracingBuildFlags flags, Ptr<gpu::AccelerationStructure> src) {
  auto dstAS = downCast<AccelerationStructure>(dst);
  auto srcAS = downCast<AccelerationStructure>(src);
  auto scratch = downCast<Buffer>(scratchBuffer);
  if (!dstAS || !scratch) {
    return;
  }

  std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> nativeGeometries;
  auto inputs = bottomLevelInputs(geometries, nativeGeometries, flags);
  inputs.Flags |=
      srcAS ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE
            : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc{};
  desc.Inputs = inputs;
  desc.SourceAccelerationStructureData = srcAS ? srcAS->getDeviceAddress() : 0;
  desc.DestAccelerationStructureData = dstAS->getDeviceAddress();
  desc.ScratchAccelerationStructureData =
      scratch->getDeviceAddress() + scratchOffset;

  ComPtr<ID3D12GraphicsCommandList4> list4;
  HRESULT hr = commandBuffer->getNative()->QueryInterface(IID_PPV_ARGS(&list4));
  if (!list4) {
    logIfFailed(hr, "D3D12: ID3D12GraphicsCommandList4 is not available");
    return;
  }
  commandBuffer->keepAlive(dstAS->getNative());
  commandBuffer->keepAlive(scratch->getNative());
  commandBuffer->getDevice()->transition(commandBuffer->getNative(),
                                         scratch.get(),
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  for (const auto &geometry : geometries) {
    if (geometry.type == gpu::RayTracingGeometryType::Aabbs) {
      auto buffer = downCast<Buffer>(geometry.aabbs.buffer);
      if (buffer) {
        commandBuffer->keepAlive(buffer->getNative());
        commandBuffer->getDevice()->transition(
            commandBuffer->getNative(), buffer.get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
      }
      continue;
    }
    auto vertexBuffer = downCast<Buffer>(geometry.triangles.vertexBuffer);
    if (vertexBuffer) {
      commandBuffer->keepAlive(vertexBuffer->getNative());
      commandBuffer->getDevice()->transition(
          commandBuffer->getNative(), vertexBuffer.get(),
          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
    auto indexBuffer = downCast<Buffer>(geometry.triangles.indexBuffer);
    if (indexBuffer) {
      commandBuffer->keepAlive(indexBuffer->getNative());
      commandBuffer->getDevice()->transition(
          commandBuffer->getNative(), indexBuffer.get(),
          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
  }
  list4->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  barrier.UAV.pResource = dstAS->getNative();
  list4->ResourceBarrier(1, &barrier);
}

void RayTracingPass::buildTopLevelAccelerationStructure(
    Ptr<gpu::AccelerationStructure> dst, const gpu::BufferBinding &instances,
    UInt32 instanceCount, Ptr<gpu::Buffer> scratchBuffer, UInt64 scratchOffset,
    gpu::RayTracingBuildFlags flags, Ptr<gpu::AccelerationStructure> src) {
  auto dstAS = downCast<AccelerationStructure>(dst);
  auto srcAS = downCast<AccelerationStructure>(src);
  auto instanceBuffer = downCast<Buffer>(instances.buffer);
  auto scratch = downCast<Buffer>(scratchBuffer);
  if (!dstAS || !instanceBuffer || !scratch) {
    return;
  }

  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
  inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
  inputs.Flags = rayTracingBuildFlagsFrom(flags);
  inputs.Flags |=
      srcAS ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE
            : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
  inputs.NumDescs = instanceCount;
  inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
  inputs.InstanceDescs = instanceBuffer->getDeviceAddress() + instances.offset;

  D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc{};
  desc.Inputs = inputs;
  desc.SourceAccelerationStructureData = srcAS ? srcAS->getDeviceAddress() : 0;
  desc.DestAccelerationStructureData = dstAS->getDeviceAddress();
  desc.ScratchAccelerationStructureData =
      scratch->getDeviceAddress() + scratchOffset;

  ComPtr<ID3D12GraphicsCommandList4> list4;
  HRESULT hr = commandBuffer->getNative()->QueryInterface(IID_PPV_ARGS(&list4));
  if (!list4) {
    logIfFailed(hr, "D3D12: ID3D12GraphicsCommandList4 is not available");
    return;
  }
  commandBuffer->keepAlive(dstAS->getNative());
  commandBuffer->keepAlive(instanceBuffer->getNative());
  commandBuffer->keepAlive(scratch->getNative());
  commandBuffer->getDevice()->transition(
      commandBuffer->getNative(), instanceBuffer.get(),
      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
  commandBuffer->getDevice()->transition(commandBuffer->getNative(),
                                         scratch.get(),
                                         D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  list4->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
  barrier.UAV.pResource = dstAS->getNative();
  list4->ResourceBarrier(1, &barrier);
}

void RayTracingPass::bindRayTracingPipeline(
    Ptr<gpu::RayTracingPipeline> rayTracingPipeline) {
  pipeline = downCast<RayTracingPipeline>(rayTracingPipeline);
  if (!pipeline) {
    return;
  }
  ComPtr<ID3D12GraphicsCommandList4> list4;
  HRESULT hr = commandBuffer->getNative()->QueryInterface(IID_PPV_ARGS(&list4));
  if (!list4) {
    logIfFailed(hr, "D3D12: ID3D12GraphicsCommandList4 is not available");
    return;
  }
  list4->SetComputeRootSignature(pipeline->getRootSignature());
  list4->SetPipelineState1(pipeline->getNative());
}

void RayTracingPass::bindAccelerationStructures(
    UInt32,
    const Array<Ptr<gpu::AccelerationStructure>> &accelerationStructures) {
  if (accelerationStructures.empty()) {
    return;
  }

  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor first{};
  for (UInt32 i = 0; i < accelerationStructures.size(); ++i) {
    auto descriptor = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      first = descriptor;
    }
    auto accelerationStructure =
        downCast<AccelerationStructure>(accelerationStructures[i]);
    if (!accelerationStructure) {
      continue;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    desc.RaytracingAccelerationStructure.Location =
        accelerationStructure->getDeviceAddress();
    device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                  descriptor.cpu);
  }
  commandBuffer->getNative()->SetComputeRootDescriptorTable(0, first.gpu);
}

void RayTracingPass::bindStorageBuffers(
    UInt32, const Array<gpu::StorageBufferBinding> &storageBuffers) {
  if (storageBuffers.empty()) {
    return;
  }

  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor first{};
  for (UInt32 i = 0; i < storageBuffers.size(); ++i) {
    auto descriptor = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      first = descriptor;
    }
    auto buffer = downCast<Buffer>(storageBuffers[i].buffer);
    if (!buffer) {
      continue;
    }
    commandBuffer->keepAlive(buffer->getNative());
    device->transition(commandBuffer->getNative(), buffer.get(),
                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = buffer->getCreateInfo().size / 4;
    desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    device->getNative()->CreateUnorderedAccessView(buffer->getNative(), nullptr,
                                                   &desc, descriptor.cpu);
  }
  commandBuffer->getNative()->SetComputeRootDescriptorTable(1, first.gpu);
}

void RayTracingPass::bindUniforms() {
  auto list = commandBuffer->getNative();
  for (UInt32 i = 0; i < 4; ++i) {
    if (auto address = commandBuffer->computeUniform(i)) {
      list->SetComputeRootConstantBufferView(i + 2, address);
    }
  }
}

void RayTracingPass::dispatchRays(
    const gpu::RayTracingShaderTableRegion &rayGeneration,
    const gpu::RayTracingShaderTableRegion &miss,
    const gpu::RayTracingShaderTableRegion &hit,
    const gpu::RayTracingShaderTableRegion &callable, UInt32 width,
    UInt32 height, UInt32 depth) {
  if (!pipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: dispatchRays called without ray tracing pipeline");
    return;
  }
  ComPtr<ID3D12GraphicsCommandList4> list4;
  HRESULT hr = commandBuffer->getNative()->QueryInterface(IID_PPV_ARGS(&list4));
  if (!list4) {
    logIfFailed(hr, "D3D12: ID3D12GraphicsCommandList4 is not available");
    return;
  }
  D3D12_DISPATCH_RAYS_DESC desc{};
  desc.RayGenerationShaderRecord.StartAddress = regionAddress(rayGeneration);
  desc.RayGenerationShaderRecord.SizeInBytes = rayGeneration.size;
  desc.MissShaderTable.StartAddress = regionAddress(miss);
  desc.MissShaderTable.SizeInBytes = miss.size;
  desc.MissShaderTable.StrideInBytes = miss.stride;
  desc.HitGroupTable.StartAddress = regionAddress(hit);
  desc.HitGroupTable.SizeInBytes = hit.size;
  desc.HitGroupTable.StrideInBytes = hit.stride;
  desc.CallableShaderTable.StartAddress = regionAddress(callable);
  desc.CallableShaderTable.SizeInBytes = callable.size;
  desc.CallableShaderTable.StrideInBytes = callable.stride;
  desc.Width = width;
  desc.Height = height;
  desc.Depth = depth;
  bindUniforms();
  list4->DispatchRays(&desc);
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
