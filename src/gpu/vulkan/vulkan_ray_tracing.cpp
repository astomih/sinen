#ifndef EMSCRIPTEN
#include "vulkan_command.hpp"
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"
#include "vulkan_resources.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <vector>

namespace sinen::gpu::vulkan {
namespace {
constexpr uint32_t kRayTracingResourceSet = 4;
constexpr uint32_t kRayTracingUniformSet = 5;
constexpr uint32_t kRayTracingAccelerationStructureBindingCount = 8;
constexpr uint32_t kRayTracingStorageBufferBaseBinding =
    kRayTracingAccelerationStructureBindingCount;

VkBuildAccelerationStructureFlagsKHR
buildFlagsFrom(gpu::RayTracingBuildFlags flags) {
  VkBuildAccelerationStructureFlagsKHR out = 0;
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::AllowUpdate)) {
    out |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::PreferFastTrace)) {
    out |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::PreferFastBuild)) {
    out |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
  }
  if (gpu::hasFlag(flags, gpu::RayTracingBuildFlags::MinimizeMemory)) {
    out |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
  }
  return out;
}

VkGeometryFlagsKHR geometryFlags(bool opaque) {
  return opaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;
}

VkIndexType indexTypeFrom(gpu::IndexElementSize size) {
  return size == gpu::IndexElementSize::Uint16 ? VK_INDEX_TYPE_UINT16
                                               : VK_INDEX_TYPE_UINT32;
}

VkShaderStageFlagBits stageFrom(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::RayGeneration:
    return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
  case ShaderStage::AnyHit:
    return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
  case ShaderStage::ClosestHit:
    return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
  case ShaderStage::Miss:
    return VK_SHADER_STAGE_MISS_BIT_KHR;
  case ShaderStage::Intersection:
    return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
  case ShaderStage::Callable:
    return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
  default:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  }
}

VkDescriptorSetLayout createDescriptorSetLayout(
    VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &bindings,
    const char *label) {
  VkDescriptorSetLayoutCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ci.bindingCount = static_cast<uint32_t>(bindings.size());
  ci.pBindings = bindings.data();

  VkDescriptorSetLayout layout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorSetLayout (%s) failed", label);
    return VK_NULL_HANDLE;
  }
  return layout;
}

std::vector<VkDescriptorSetLayoutBinding>
descriptorBindings(uint32_t count, VkDescriptorType type) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = i;
    binding.descriptorType = type;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    bindings.push_back(binding);
  }
  return bindings;
}

std::vector<VkDescriptorSetLayoutBinding> rayTracingResourceBindings() {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(kRayTracingAccelerationStructureBindingCount + 32);
  for (uint32_t i = 0; i < kRayTracingAccelerationStructureBindingCount; ++i) {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = i;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    bindings.push_back(binding);
  }
  for (uint32_t i = 0; i < 32; ++i) {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = kRayTracingStorageBufferBaseBinding + i;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    bindings.push_back(binding);
  }
  return bindings;
}

VkAccelerationStructureGeometryKHR
geometryFrom(const gpu::RayTracingGeometry &geometry) {
  VkAccelerationStructureGeometryKHR out{};
  out.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  if (geometry.type == gpu::RayTracingGeometryType::Aabbs) {
    auto buffer = downCast<Buffer>(geometry.aabbs.buffer);
    out.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
    out.flags = geometryFlags(geometry.aabbs.opaque);
    out.geometry.aabbs.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
    out.geometry.aabbs.data.deviceAddress =
        buffer ? buffer->getDeviceAddress() + geometry.aabbs.offset : 0;
    out.geometry.aabbs.stride = geometry.aabbs.stride;
    return out;
  }

  auto vertexBuffer = downCast<Buffer>(geometry.triangles.vertexBuffer);
  auto indexBuffer = downCast<Buffer>(geometry.triangles.indexBuffer);
  out.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
  out.flags = geometryFlags(geometry.triangles.opaque);
  auto &triangles = out.geometry.triangles;
  triangles.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
  triangles.vertexFormat =
      convert::vertexElementFormatFrom(geometry.triangles.vertexFormat);
  triangles.vertexData.deviceAddress =
      vertexBuffer
          ? vertexBuffer->getDeviceAddress() + geometry.triangles.vertexOffset
          : 0;
  triangles.vertexStride = geometry.triangles.vertexStride;
  triangles.maxVertex = geometry.triangles.vertexCount;
  triangles.indexType = indexBuffer
                            ? indexTypeFrom(geometry.triangles.indexElementSize)
                            : VK_INDEX_TYPE_NONE_KHR;
  triangles.indexData.deviceAddress =
      indexBuffer
          ? indexBuffer->getDeviceAddress() + geometry.triangles.indexOffset
          : 0;
  return out;
}

uint32_t primitiveCountFrom(const gpu::RayTracingGeometry &geometry) {
  if (geometry.type == gpu::RayTracingGeometryType::Aabbs) {
    return geometry.aabbs.count;
  }
  if (geometry.triangles.indexCount > 0) {
    return geometry.triangles.indexCount / 3;
  }
  return geometry.triangles.vertexCount / 3;
}

void collectBottomLevelGeometry(
    const Array<gpu::RayTracingGeometry> &geometries,
    std::vector<VkAccelerationStructureGeometryKHR> &nativeGeometries,
    std::vector<uint32_t> &primitiveCounts) {
  nativeGeometries.clear();
  primitiveCounts.clear();
  nativeGeometries.reserve(geometries.size());
  primitiveCounts.reserve(geometries.size());
  for (const auto &geometry : geometries) {
    nativeGeometries.push_back(geometryFrom(geometry));
    primitiveCounts.push_back(primitiveCountFrom(geometry));
  }
}

VkDeviceAddress regionAddress(const gpu::RayTracingShaderTableRegion &region) {
  auto buffer = downCast<Buffer>(region.buffer);
  return buffer ? buffer->getDeviceAddress() + region.offset : 0;
}

uint32_t shaderGroupIndex(const std::vector<uint32_t> &indices,
                          uint32_t shaderIndex) {
  return shaderIndex < indices.size() ? indices[shaderIndex]
                                      : VK_SHADER_UNUSED_KHR;
}
} // namespace

gpu::RayTracingAccelerationStructureBuildSizes
Device::getBottomLevelAccelerationStructureBuildSizes(
    const Array<gpu::RayTracingGeometry> &geometries,
    gpu::RayTracingBuildFlags flags) {
  if (!rayTracingSupported) {
    return {};
  }
  std::vector<VkAccelerationStructureGeometryKHR> nativeGeometries;
  std::vector<uint32_t> primitiveCounts;
  collectBottomLevelGeometry(geometries, nativeGeometries, primitiveCounts);

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
  buildInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  buildInfo.flags = buildFlagsFrom(flags);
  buildInfo.geometryCount = static_cast<uint32_t>(nativeGeometries.size());
  buildInfo.pGeometries = nativeGeometries.data();

  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
  sizeInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  vkGetAccelerationStructureBuildSizesKHR(
      device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
      primitiveCounts.data(), &sizeInfo);
  return {sizeInfo.accelerationStructureSize, sizeInfo.buildScratchSize,
          sizeInfo.updateScratchSize};
}

gpu::RayTracingAccelerationStructureBuildSizes
Device::getTopLevelAccelerationStructureBuildSizes(
    UInt32 instanceCount, gpu::RayTracingBuildFlags flags) {
  if (!rayTracingSupported) {
    return {};
  }
  VkAccelerationStructureGeometryKHR geometry{};
  geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
  geometry.geometry.instances.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
  buildInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
  buildInfo.flags = buildFlagsFrom(flags);
  buildInfo.geometryCount = 1;
  buildInfo.pGeometries = &geometry;

  VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
  sizeInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
  vkGetAccelerationStructureBuildSizesKHR(
      device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
      &instanceCount, &sizeInfo);
  return {sizeInfo.accelerationStructureSize, sizeInfo.buildScratchSize,
          sizeInfo.updateScratchSize};
}

Ptr<gpu::AccelerationStructure> Device::createAccelerationStructure(
    const gpu::AccelerationStructure::CreateInfo &createInfo) {
  if (!rayTracingSupported || createInfo.size == 0) {
    return nullptr;
  }

  VkBufferCreateInfo bufferCI{};
  bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCI.size = createInfo.size;
  bufferCI.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
  bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocCI{};
  allocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  if (vmaCreateBuffer(vmaAllocator, &bufferCI, &allocCI, &buffer, &allocation,
                      nullptr) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vmaCreateBuffer (acceleration structure) failed");
    return nullptr;
  }

  VkAccelerationStructureCreateInfoKHR asCI{};
  asCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
  asCI.buffer = buffer;
  asCI.size = createInfo.size;
  asCI.type = createInfo.type == gpu::AccelerationStructure::Type::TopLevel
                  ? VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
                  : VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

  VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;
  if (vkCreateAccelerationStructureKHR(device, &asCI, nullptr,
                                       &accelerationStructure) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateAccelerationStructureKHR failed");
    vmaDestroyBuffer(vmaAllocator, buffer, allocation);
    return nullptr;
  }

  VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
  addressInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
  addressInfo.accelerationStructure = accelerationStructure;
  VkDeviceAddress address =
      vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);

  return makePtr<AccelerationStructure>(createInfo.allocator, createInfo, *this,
                                        buffer, allocation,
                                        accelerationStructure, address);
}

Ptr<gpu::RayTracingPipeline> Device::createRayTracingPipeline(
    const gpu::RayTracingPipeline::CreateInfo &createInfo) {
  if (!rayTracingSupported) {
    return nullptr;
  }

  VkDescriptorSetLayout emptySetLayout =
      createDescriptorSetLayout(device, {}, "ray tracing empty");
  VkDescriptorSetLayout resourceSetLayout = createDescriptorSetLayout(
      device, rayTracingResourceBindings(), "ray tracing resources");
  VkDescriptorSetLayout uniformSetLayout = createDescriptorSetLayout(
      device, descriptorBindings(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC),
      "ray tracing uniforms");
  if (!emptySetLayout || !resourceSetLayout || !uniformSetLayout) {
    if (uniformSetLayout)
      vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    if (resourceSetLayout)
      vkDestroyDescriptorSetLayout(device, resourceSetLayout, nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  std::array<VkDescriptorSetLayout, 6> setLayouts = {
      emptySetLayout, emptySetLayout,    emptySetLayout,
      emptySetLayout, resourceSetLayout, uniformSetLayout};
  VkPipelineLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  layoutCI.pSetLayouts = setLayouts.data();
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreatePipelineLayout (ray tracing) failed");
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, resourceSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  std::vector<VkPipelineShaderStageCreateInfo> stages;
  stages.reserve(createInfo.shaders.size());
  std::vector<uint32_t> stageIndexByShader(createInfo.shaders.size(),
                                           VK_SHADER_UNUSED_KHR);
  for (size_t i = 0; i < createInfo.shaders.size(); ++i) {
    const auto &shaderInfo = createInfo.shaders[i];
    auto shader = downCast<Shader>(shaderInfo.shader);
    if (!shader) {
      continue;
    }
    VkPipelineShaderStageCreateInfo stage{};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = stageFrom(shader->getStage());
    stage.module = shader->getNative();
    stage.pName = shader->getEntrypoint();
    stageIndexByShader[i] = static_cast<uint32_t>(stages.size());
    stages.push_back(stage);
  }

  std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
  groups.reserve(createInfo.shaders.size() + createInfo.hitGroups.size());
  for (size_t i = 0; i < createInfo.shaders.size(); ++i) {
    auto shader = downCast<Shader>(createInfo.shaders[i].shader);
    if (!shader) {
      continue;
    }
    const auto stage = shader->getStage();
    if (stage == ShaderStage::AnyHit || stage == ShaderStage::ClosestHit ||
        stage == ShaderStage::Intersection) {
      continue;
    }
    VkRayTracingShaderGroupCreateInfoKHR group{};
    group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader = stageIndexByShader[i];
    group.closestHitShader = VK_SHADER_UNUSED_KHR;
    group.anyHitShader = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;
    groups.push_back(group);
  }

  for (const auto &hitGroup : createInfo.hitGroups) {
    VkRayTracingShaderGroupCreateInfoKHR group{};
    group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group.type =
        hitGroup.intersectionShaderIndex == UINT32_MAX
            ? VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR
            : VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    group.generalShader = VK_SHADER_UNUSED_KHR;
    group.closestHitShader =
        hitGroup.closestHitShaderIndex == UINT32_MAX
            ? VK_SHADER_UNUSED_KHR
            : shaderGroupIndex(stageIndexByShader,
                               hitGroup.closestHitShaderIndex);
    group.anyHitShader =
        hitGroup.anyHitShaderIndex == UINT32_MAX
            ? VK_SHADER_UNUSED_KHR
            : shaderGroupIndex(stageIndexByShader, hitGroup.anyHitShaderIndex);
    group.intersectionShader =
        hitGroup.intersectionShaderIndex == UINT32_MAX
            ? VK_SHADER_UNUSED_KHR
            : shaderGroupIndex(stageIndexByShader,
                               hitGroup.intersectionShaderIndex);
    groups.push_back(group);
  }

  VkRayTracingPipelineCreateInfoKHR pipelineCI{};
  pipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
  pipelineCI.stageCount = static_cast<uint32_t>(stages.size());
  pipelineCI.pStages = stages.data();
  pipelineCI.groupCount = static_cast<uint32_t>(groups.size());
  pipelineCI.pGroups = groups.data();
  pipelineCI.maxPipelineRayRecursionDepth = createInfo.maxRecursionDepth;
  pipelineCI.layout = pipelineLayout;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                     &pipelineCI, nullptr,
                                     &pipeline) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateRayTracingPipelinesKHR failed");
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, resourceSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  RayTracingPipeline::LayoutInfo layoutInfo{};
  layoutInfo.emptySetLayout = emptySetLayout;
  layoutInfo.resourceSetLayout = resourceSetLayout;
  layoutInfo.uniformSetLayout = uniformSetLayout;
  layoutInfo.pipelineLayout = pipelineLayout;
  return makePtr<RayTracingPipeline>(createInfo.allocator, createInfo, *this,
                                     pipeline, layoutInfo);
}

RayTracingPass::RayTracingPass(Device &device, CommandBuffer &commandBuffer)
    : device(device), commandBuffer(commandBuffer),
      cmd(commandBuffer.getNative()),
      accelerationStructures(commandBuffer.getCreateInfo().allocator),
      storageBuffers(commandBuffer.getCreateInfo().allocator) {}

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

  std::vector<VkAccelerationStructureGeometryKHR> nativeGeometries;
  std::vector<uint32_t> primitiveCounts;
  collectBottomLevelGeometry(geometries, nativeGeometries, primitiveCounts);
  std::vector<VkAccelerationStructureBuildRangeInfoKHR> ranges(
      nativeGeometries.size());
  std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> rangePtrs(
      nativeGeometries.size());
  for (size_t i = 0; i < ranges.size(); ++i) {
    ranges[i].primitiveCount = primitiveCounts[i];
    rangePtrs[i] = &ranges[i];
  }

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
  buildInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
  buildInfo.flags = buildFlagsFrom(flags);
  buildInfo.mode = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR
                         : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
  buildInfo.srcAccelerationStructure =
      srcAS ? srcAS->getNative() : VK_NULL_HANDLE;
  buildInfo.dstAccelerationStructure = dstAS->getNative();
  buildInfo.geometryCount = static_cast<uint32_t>(nativeGeometries.size());
  buildInfo.pGeometries = nativeGeometries.data();
  buildInfo.scratchData.deviceAddress =
      scratch->getDeviceAddress() + scratchOffset;

  commandBuffer.keepAlive(dst);
  commandBuffer.keepAlive(scratchBuffer);
  vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, rangePtrs.data());

  VkMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
  barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                          VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
  vkCmdPipelineBarrier(cmd,
                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                       0, 1, &barrier, 0, nullptr, 0, nullptr);
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

  VkAccelerationStructureGeometryKHR geometry{};
  geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
  geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
  geometry.geometry.instances.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
  geometry.geometry.instances.data.deviceAddress =
      instanceBuffer->getDeviceAddress() + instances.offset;

  VkAccelerationStructureBuildRangeInfoKHR range{};
  range.primitiveCount = instanceCount;
  const VkAccelerationStructureBuildRangeInfoKHR *rangePtr = &range;

  VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
  buildInfo.sType =
      VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
  buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
  buildInfo.flags = buildFlagsFrom(flags);
  buildInfo.mode = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR
                         : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
  buildInfo.srcAccelerationStructure =
      srcAS ? srcAS->getNative() : VK_NULL_HANDLE;
  buildInfo.dstAccelerationStructure = dstAS->getNative();
  buildInfo.geometryCount = 1;
  buildInfo.pGeometries = &geometry;
  buildInfo.scratchData.deviceAddress =
      scratch->getDeviceAddress() + scratchOffset;

  commandBuffer.keepAlive(dst);
  commandBuffer.keepAlive(instances.buffer);
  commandBuffer.keepAlive(scratchBuffer);
  vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &rangePtr);

  VkMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
  barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                          VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
  vkCmdPipelineBarrier(cmd,
                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                       VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                       0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void RayTracingPass::bindRayTracingPipeline(
    Ptr<gpu::RayTracingPipeline> pipeline) {
  boundPipeline = downCast<RayTracingPipeline>(pipeline);
  if (!boundPipeline) {
    return;
  }
  commandBuffer.keepAlive(pipeline);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                    boundPipeline->getNative());
}

void RayTracingPass::bindAccelerationStructures(
    UInt32 startSlot,
    const Array<Ptr<gpu::AccelerationStructure>> &accelerationStructures) {
  accelerationStructureStartSlot = startSlot;
  this->accelerationStructures = Array<Ptr<gpu::AccelerationStructure>>(
      accelerationStructures, commandBuffer.getCreateInfo().allocator);
}

void RayTracingPass::bindStorageBuffers(
    UInt32 startSlot, const Array<gpu::StorageBufferBinding> &storageBuffers) {
  storageBufferStartSlot = startSlot;
  this->storageBuffers = Array<gpu::StorageBufferBinding>(
      storageBuffers, commandBuffer.getCreateInfo().allocator);
}

void RayTracingPass::bindDescriptorSets() {
  if (!boundPipeline) {
    return;
  }

  const auto &layoutInfo = boundPipeline->getLayoutInfo();
  if ((!accelerationStructures.empty() || !storageBuffers.empty()) &&
      layoutInfo.resourceSetLayout != VK_NULL_HANDLE) {
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = commandBuffer.getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutInfo.resourceSetLayout;
    if (vkAllocateDescriptorSets(device.getVkDevice(), &allocInfo, &set) ==
        VK_SUCCESS) {
      std::vector<VkAccelerationStructureKHR> handles;
      handles.reserve(accelerationStructures.size());
      for (auto &as : accelerationStructures) {
        auto native = downCast<AccelerationStructure>(as);
        handles.push_back(native ? native->getNative() : VK_NULL_HANDLE);
        commandBuffer.keepAlive(as);
      }

      std::vector<VkWriteDescriptorSetAccelerationStructureKHR> asInfos(
          handles.size());
      std::vector<VkDescriptorBufferInfo> bufferInfos(storageBuffers.size());
      std::vector<VkWriteDescriptorSet> writes;
      writes.reserve(handles.size() + storageBuffers.size());
      for (size_t i = 0; i < handles.size(); ++i) {
        asInfos[i].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        asInfos[i].accelerationStructureCount = 1;
        asInfos[i].pAccelerationStructures = &handles[i];
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = &asInfos[i];
        write.dstSet = set;
        write.dstBinding =
            accelerationStructureStartSlot + static_cast<uint32_t>(i);
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        writes.push_back(write);
      }
      for (size_t i = 0; i < storageBuffers.size(); ++i) {
        auto buffer = downCast<Buffer>(storageBuffers[i].buffer);
        if (!buffer) {
          continue;
        }
        commandBuffer.keepAlive(storageBuffers[i].buffer);
        bufferInfos[i].buffer = buffer->getNative();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = buffer->getCreateInfo().size;
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set;
        write.dstBinding = kRayTracingStorageBufferBaseBinding +
                           storageBufferStartSlot + static_cast<uint32_t>(i);
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.pBufferInfo = &bufferInfos[i];
        writes.push_back(write);
      }
      vkUpdateDescriptorSets(device.getVkDevice(),
                             static_cast<uint32_t>(writes.size()),
                             writes.data(), 0, nullptr);
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                              layoutInfo.pipelineLayout, kRayTracingResourceSet,
                              1, &set, 0, nullptr);
    }
  }

  uint32_t uniformOffsets[4]{};
  for (uint32_t i = 0; i < 4; ++i) {
    uniformOffsets[i] = commandBuffer.getComputeUniformSlotOffset(i);
  }
  constexpr uint32_t uniformCount = 4;
  if (uniformCount > 0 && layoutInfo.uniformSetLayout != VK_NULL_HANDLE) {
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = commandBuffer.getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layoutInfo.uniformSetLayout;
    if (vkAllocateDescriptorSets(device.getVkDevice(), &allocInfo, &set) ==
        VK_SUCCESS) {
      std::vector<VkDescriptorBufferInfo> bufferInfos(uniformCount);
      std::vector<VkWriteDescriptorSet> writes(uniformCount);
      for (uint32_t i = 0; i < uniformCount; ++i) {
        bufferInfos[i].buffer = commandBuffer.getUniformBuffer();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = commandBuffer.getUniformRange();
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = set;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writes[i].pBufferInfo = &bufferInfos[i];
      }
      vkUpdateDescriptorSets(device.getVkDevice(), uniformCount, writes.data(),
                             0, nullptr);
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                              layoutInfo.pipelineLayout, kRayTracingUniformSet,
                              1, &set, uniformCount, uniformOffsets);
    }
  }
}

void RayTracingPass::dispatchRays(
    const gpu::RayTracingShaderTableRegion &rayGeneration,
    const gpu::RayTracingShaderTableRegion &miss,
    const gpu::RayTracingShaderTableRegion &hit,
    const gpu::RayTracingShaderTableRegion &callable, UInt32 width,
    UInt32 height, UInt32 depth) {
  if (!boundPipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: dispatchRays called without ray tracing pipeline");
    return;
  }
  VkStridedDeviceAddressRegionKHR raygenRegion{};
  raygenRegion.deviceAddress = regionAddress(rayGeneration);
  raygenRegion.size = rayGeneration.size;
  raygenRegion.stride = rayGeneration.stride;
  VkStridedDeviceAddressRegionKHR missRegion{};
  missRegion.deviceAddress = regionAddress(miss);
  missRegion.size = miss.size;
  missRegion.stride = miss.stride;
  VkStridedDeviceAddressRegionKHR hitRegion{};
  hitRegion.deviceAddress = regionAddress(hit);
  hitRegion.size = hit.size;
  hitRegion.stride = hit.stride;
  VkStridedDeviceAddressRegionKHR callableRegion{};
  callableRegion.deviceAddress = regionAddress(callable);
  callableRegion.size = callable.size;
  callableRegion.stride = callable.stride;

  bindDescriptorSets();
  vkCmdTraceRaysKHR(cmd, &raygenRegion, &missRegion, &hitRegion,
                    &callableRegion, width, height, depth);
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
