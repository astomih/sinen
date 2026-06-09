#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <gpu/compute/compute.hpp>
#include <gpu/compute/compute_buffer.hpp>
#include <gpu/raytracing/raytracing.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

namespace sinen {
namespace {
UInt32 optFieldU32(lua_State *L, int tableIndex, const char *name,
                   UInt32 defaultValue) {
  lua_getfield(L, tableIndex, name);
  UInt32 value = defaultValue;
  if (!lua_isnil(L, -1)) {
    value = static_cast<UInt32>(luaL_checkinteger(L, -1));
  }
  lua_pop(L, 1);
  return value;
}

UInt64 optFieldU64(lua_State *L, int tableIndex, const char *name,
                   UInt64 defaultValue) {
  lua_getfield(L, tableIndex, name);
  UInt64 value = defaultValue;
  if (!lua_isnil(L, -1)) {
    value = static_cast<UInt64>(luaL_checknumber(L, -1));
  }
  lua_pop(L, 1);
  return value;
}

bool optFieldBool(lua_State *L, int tableIndex, const char *name,
                  bool defaultValue) {
  lua_getfield(L, tableIndex, name);
  bool value = defaultValue;
  if (!lua_isnil(L, -1)) {
    value = lua_toboolean(L, -1) != 0;
  }
  lua_pop(L, 1);
  return value;
}

Ptr<gpu::Buffer> computeBufferField(lua_State *L, int tableIndex,
                                    const char *name, bool required) {
  lua_getfield(L, tableIndex, name);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    if (required) {
      luaL_error(L, "missing ComputeBuffer field '%s'", name);
    }
    return nullptr;
  }
  auto &buffer = udPtr<ComputeBuffer>(L, -1);
  auto raw = buffer->getRaw();
  lua_pop(L, 1);
  return raw;
}

gpu::RayTracingShaderTableRegion readRegion(lua_State *L, int tableIndex,
                                            const char *name,
                                            const Ptr<gpu::Buffer> &shaderTable,
                                            UInt32 handleSize,
                                            UInt64 defaultOffset) {
  gpu::RayTracingShaderTableRegion region{};
  region.buffer = shaderTable;
  region.offset = defaultOffset;
  region.size = handleSize;
  region.stride = handleSize;

  lua_getfield(L, tableIndex, name);
  if (lua_istable(L, -1)) {
    const int regionIndex = lua_gettop(L);
    region.offset = optFieldU64(L, regionIndex, "offset", region.offset);
    region.size = optFieldU64(L, regionIndex, "size", region.size);
    region.stride = optFieldU64(L, regionIndex, "stride", region.stride);
  }
  lua_pop(L, 1);
  return region;
}

void fillIdentityTransform(gpu::RayTracingInstance &instance) {
  instance.transform[0][0] = 1.0f;
  instance.transform[0][1] = 0.0f;
  instance.transform[0][2] = 0.0f;
  instance.transform[0][3] = 0.0f;
  instance.transform[1][0] = 0.0f;
  instance.transform[1][1] = 1.0f;
  instance.transform[1][2] = 0.0f;
  instance.transform[1][3] = 0.0f;
  instance.transform[2][0] = 0.0f;
  instance.transform[2][1] = 0.0f;
  instance.transform[2][2] = 1.0f;
  instance.transform[2][3] = 0.0f;
}

void readTransform(lua_State *L, int tableIndex,
                   gpu::RayTracingInstance &instance) {
  fillIdentityTransform(instance);
  lua_getfield(L, tableIndex, "transform");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return;
  }
  for (int i = 0; i < 12; ++i) {
    lua_rawgeti(L, -1, i + 1);
    instance.transform[i / 4][i % 4] =
        static_cast<float>(luaL_checknumber(L, -1));
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

bool requireRayTracingSupport(lua_State *L, const char *functionName) {
  if (Graphics::getDevice()->supportsRayTracing()) {
    return true;
  }
  luaLError2(L, "%s called on a device without ray tracing support",
             functionName);
  return false;
}

bool requireRayQuerySupport(lua_State *L, const char *functionName) {
  if (Graphics::getDevice()->supportsRayQuery()) {
    return true;
  }
  luaLError2(L, "%s called on a device without ray query support",
             functionName);
  return false;
}
} // namespace

static int lRaytracingIsSupported(lua_State *L) {
  lua_pushboolean(L, Graphics::getDevice()->supportsRayTracing());
  return 1;
}

static int lRaytracingIsRayQuerySupported(lua_State *L) {
  lua_pushboolean(L, Graphics::getDevice()->supportsRayQuery());
  return 1;
}

static int lRaytracingSetComputeAccelerationStructure(lua_State *L) {
  if (!requireRayQuerySupport(L,
                              "Raytracing.setComputeAccelerationStructure")) {
    return 0;
  }
  const auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &as = udPtr<RaytracingAccelerationStructure>(L, 2);
  Compute::setAccelerationStructure(slot, as->getRaw());
  return 0;
}

static int lRaytracingResetComputeAccelerationStructure(lua_State *L) {
  if (!requireRayQuerySupport(L,
                              "Raytracing.resetComputeAccelerationStructure")) {
    return 0;
  }
  const auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Compute::resetAccelerationStructure(slot);
  return 0;
}

static int lRaytracingResetAllComputeAccelerationStructures(lua_State *L) {
  if (!requireRayQuerySupport(
          L, "Raytracing.resetAllComputeAccelerationStructures")) {
    return 0;
  }
  (void)L;
  Compute::resetAllAccelerationStructures();
  return 0;
}

static int lRaytracingSetGraphicsAccelerationStructure(lua_State *L) {
  if (!requireRayQuerySupport(L,
                              "Raytracing.setGraphicsAccelerationStructure")) {
    return 0;
  }
  const auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &as = udPtr<RaytracingAccelerationStructure>(L, 2);
  Graphics::setAccelerationStructure(slot, as->getRaw());
  return 0;
}

static int lRaytracingResetGraphicsAccelerationStructure(lua_State *L) {
  if (!requireRayQuerySupport(
          L, "Raytracing.resetGraphicsAccelerationStructure")) {
    return 0;
  }
  const auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Graphics::resetAccelerationStructure(slot);
  return 0;
}

static int lRaytracingResetAllGraphicsAccelerationStructures(lua_State *L) {
  if (!requireRayQuerySupport(
          L, "Raytracing.resetAllGraphicsAccelerationStructures")) {
    return 0;
  }
  (void)L;
  Graphics::resetAllAccelerationStructures();
  return 0;
}

static int lRaytracingPipelineNew(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.new")) {
    return 0;
  }
  udPushPtr<RaytracingPipeline>(L, makePtr<RaytracingPipeline>());
  return 1;
}

static int lRaytracingPipelineAddShader(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.addShader")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  auto &shader = udPtr<Shader>(L, 2);
  const char *exportName = luaL_optstring(L, 3, nullptr);
  pipeline->addShader(*shader, exportName);
  return 0;
}

static int lRaytracingPipelineAddHitGroup(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.addHitGroup")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  const char *exportName = luaL_checkstring(L, 2);
  const auto closestHit =
      static_cast<UInt32>(luaL_optinteger(L, 3, UINT32_MAX));
  const auto anyHit = static_cast<UInt32>(luaL_optinteger(L, 4, UINT32_MAX));
  const auto intersection =
      static_cast<UInt32>(luaL_optinteger(L, 5, UINT32_MAX));
  pipeline->addHitGroup(exportName, closestHit, anyHit, intersection);
  return 0;
}

static int lRaytracingPipelineSetMaxPayloadSize(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.setMaxPayloadSize")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  pipeline->setMaxPayloadSize(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  return 0;
}

static int lRaytracingPipelineSetMaxAttributeSize(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.setMaxAttributeSize")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  pipeline->setMaxAttributeSize(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  return 0;
}

static int lRaytracingPipelineSetMaxRecursionDepth(lua_State *L) {
  if (!requireRayTracingSupport(L,
                                "Raytracing.Pipeline.setMaxRecursionDepth")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  pipeline->setMaxRecursionDepth(static_cast<UInt32>(luaL_checkinteger(L, 2)));
  return 0;
}

static int lRaytracingPipelineBuild(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.build")) {
    return 0;
  }
  udPtr<RaytracingPipeline>(L, 1)->build();
  return 0;
}

static int lRaytracingPipelineIsReady(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.Pipeline.isReady")) {
    return 0;
  }
  lua_pushboolean(L, udPtr<RaytracingPipeline>(L, 1)->isReady());
  return 1;
}

static int lRaytracingPipelineGetShaderGroupHandleSize(lua_State *L) {
  if (!requireRayTracingSupport(
          L, "Raytracing.Pipeline.getShaderGroupHandleSize")) {
    return 0;
  }
  lua_pushinteger(L,
                  udPtr<RaytracingPipeline>(L, 1)->getShaderGroupHandleSize());
  return 1;
}

static int lRaytracingPipelineGetShaderGroupHandles(lua_State *L) {
  if (!requireRayTracingSupport(L,
                                "Raytracing.Pipeline.getShaderGroupHandles")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  auto first = static_cast<UInt32>(luaL_checkinteger(L, 2));
  auto count = static_cast<UInt32>(luaL_checkinteger(L, 3));
  udNewOwned<Buffer>(L, pipeline->getShaderGroupHandles(first, count));
  return 1;
}

static int lRaytracingAccelerationStructureGetDeviceAddress(lua_State *L) {
  if (!requireRayTracingSupport(
          L, "Raytracing.AccelerationStructure.getDeviceAddress")) {
    return 0;
  }
  auto &as = udPtr<RaytracingAccelerationStructure>(L, 1);
  lua_pushnumber(L, static_cast<double>(as->getDeviceAddress()));
  return 1;
}

static int lRaytracingCreateBottomLevel(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.createBottomLevel")) {
    return 0;
  }
  luaL_checktype(L, 1, LUA_TTABLE);
  auto flags = static_cast<gpu::RayTracingBuildFlags>(
      luaL_optinteger(L, 2,
                      static_cast<lua_Integer>(static_cast<UInt32>(
                          gpu::RayTracingBuildFlags::PreferFastTrace))));

  auto allocator = GlobalAllocator::get();
  Array<gpu::RayTracingGeometry> geometries(allocator);
  const size_t count = lua_objlen(L, 1);
  geometries.reserve(count);
  for (size_t i = 1; i <= count; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));
    luaL_checktype(L, -1, LUA_TTABLE);
    const int geometryIndex = lua_gettop(L);

    gpu::RayTracingGeometry geometry{};
    lua_getfield(L, geometryIndex, "aabbBuffer");
    const bool isAabb = !lua_isnil(L, -1);
    lua_pop(L, 1);
    if (isAabb) {
      geometry.type = gpu::RayTracingGeometryType::Aabbs;
      geometry.aabbs.buffer =
          computeBufferField(L, geometryIndex, "aabbBuffer", true);
      geometry.aabbs.offset = optFieldU64(L, geometryIndex, "offset", 0);
      geometry.aabbs.stride = optFieldU32(L, geometryIndex, "stride", 24);
      geometry.aabbs.count = optFieldU32(L, geometryIndex, "count", 0);
      geometry.aabbs.opaque = optFieldBool(L, geometryIndex, "opaque", true);
    } else {
      geometry.type = gpu::RayTracingGeometryType::Triangles;
      geometry.triangles.vertexBuffer =
          computeBufferField(L, geometryIndex, "vertexBuffer", true);
      geometry.triangles.vertexOffset =
          optFieldU64(L, geometryIndex, "vertexOffset", 0);
      geometry.triangles.vertexStride =
          optFieldU32(L, geometryIndex, "vertexStride", sizeof(float) * 3);
      geometry.triangles.vertexCount =
          optFieldU32(L, geometryIndex, "vertexCount", 0);
      geometry.triangles.indexBuffer =
          computeBufferField(L, geometryIndex, "indexBuffer", false);
      geometry.triangles.indexOffset =
          optFieldU64(L, geometryIndex, "indexOffset", 0);
      geometry.triangles.indexCount =
          optFieldU32(L, geometryIndex, "indexCount", 0);
      geometry.triangles.opaque =
          optFieldBool(L, geometryIndex, "opaque", true);
    }
    geometries.push_back(geometry);
    lua_pop(L, 1);
  }

  String error;
  auto accelerationStructure =
      createBottomLevelAccelerationStructure(geometries, flags, &error);
  if (!accelerationStructure) {
    return luaLError2(L, "%s", error.c_str());
  }
  udPushPtr<RaytracingAccelerationStructure>(L, accelerationStructure);
  return 1;
}

static int lRaytracingCreateTopLevel(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.createTopLevel")) {
    return 0;
  }
  luaL_checktype(L, 1, LUA_TTABLE);
  auto flags = static_cast<gpu::RayTracingBuildFlags>(
      luaL_optinteger(L, 2,
                      static_cast<lua_Integer>(static_cast<UInt32>(
                          gpu::RayTracingBuildFlags::PreferFastTrace))));

  auto allocator = GlobalAllocator::get();
  Array<gpu::RayTracingInstance> instances(allocator);
  const size_t count = lua_objlen(L, 1);
  instances.reserve(count);
  for (size_t i = 1; i <= count; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));
    luaL_checktype(L, -1, LUA_TTABLE);
    const int instanceIndex = lua_gettop(L);

    gpu::RayTracingInstance instance{};
    readTransform(L, instanceIndex, instance);
    instance.instanceCustomIndex =
        optFieldU32(L, instanceIndex, "instanceCustomIndex", 0) & 0xffffffu;
    instance.mask = optFieldU32(L, instanceIndex, "mask", 0xff) & 0xffu;
    instance.shaderBindingTableRecordOffset =
        optFieldU32(L, instanceIndex, "shaderBindingTableRecordOffset", 0) &
        0xffffffu;
    instance.flags = optFieldU32(L, instanceIndex, "flags", 0) & 0xffu;

    lua_getfield(L, instanceIndex, "accelerationStructure");
    auto &as = udPtr<RaytracingAccelerationStructure>(L, -1);
    instance.accelerationStructureAddress = as->getDeviceAddress();
    lua_pop(L, 1);

    instances.push_back(instance);
    lua_pop(L, 1);
  }

  String error;
  auto accelerationStructure =
      createTopLevelAccelerationStructure(instances, flags, &error);
  if (!accelerationStructure) {
    return luaLError2(L, "%s", error.c_str());
  }
  udPushPtr<RaytracingAccelerationStructure>(L, accelerationStructure);
  return 1;
}

static int lRaytracingDispatch(lua_State *L) {
  if (!requireRayTracingSupport(L, "Raytracing.dispatch")) {
    return 0;
  }
  auto &pipeline = udPtr<RaytracingPipeline>(L, 1);
  luaL_checktype(L, 2, LUA_TTABLE);
  const int descIndex = 2;

  lua_getfield(L, descIndex, "shaderTable");
  auto &shaderTable = udPtr<ComputeBuffer>(L, -1);
  auto shaderTableRaw = shaderTable->getRaw();
  lua_pop(L, 1);

  RaytracingDispatchInfo info;
  info.width = optFieldU32(L, descIndex, "width", 1);
  info.height = optFieldU32(L, descIndex, "height", 1);
  info.depth = optFieldU32(L, descIndex, "depth", 1);
  const UInt32 handleSize = pipeline->getShaderGroupHandleSize();
  info.rayGeneration =
      readRegion(L, descIndex, "rayGeneration", shaderTableRaw, handleSize, 0);
  info.miss =
      readRegion(L, descIndex, "miss", shaderTableRaw, handleSize, handleSize);
  info.hit = readRegion(L, descIndex, "hit", shaderTableRaw, handleSize,
                        handleSize * 2);
  info.callable = readRegion(L, descIndex, "callable", shaderTableRaw, 0, 0);

  lua_getfield(L, descIndex, "uniforms");
  if (lua_istable(L, -1)) {
    const int uniformsIndex = lua_gettop(L);
    const size_t count = lua_objlen(L, uniformsIndex);
    info.uniforms.reserve(count);
    for (size_t i = 1; i <= count; ++i) {
      lua_rawgeti(L, uniformsIndex, static_cast<lua_Integer>(i));
      auto &buffer = udValue<Buffer>(L, -1);
      info.uniforms.push_back(buffer);
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);

  lua_getfield(L, descIndex, "accelerationStructures");
  if (lua_istable(L, -1)) {
    const int asIndex = lua_gettop(L);
    const size_t count = lua_objlen(L, asIndex);
    info.accelerationStructures.reserve(count);
    for (size_t i = 1; i <= count; ++i) {
      lua_rawgeti(L, asIndex, static_cast<lua_Integer>(i));
      auto &as = udPtr<RaytracingAccelerationStructure>(L, -1);
      info.accelerationStructures.push_back(as->getRaw());
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);

  lua_getfield(L, descIndex, "storageBuffers");
  if (lua_istable(L, -1)) {
    const int storageIndex = lua_gettop(L);
    const size_t count = lua_objlen(L, storageIndex);
    info.storageBuffers.resize(count);
    for (size_t i = 1; i <= count; ++i) {
      lua_rawgeti(L, storageIndex, static_cast<lua_Integer>(i));
      auto &buffer = udPtr<ComputeBuffer>(L, -1);
      info.storageBuffers[i - 1].buffer = buffer->getRaw();
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);

  String error;
  if (!dispatchRays(*pipeline, info, &error)) {
    return luaLError2(L, "%s", error.c_str());
  }
  return 0;
}

void registerRaytracing(lua_State *L) {
  luaL_newmetatable(L, RaytracingAccelerationStructure::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRaytracingAccelerationStructureGetDeviceAddress);
  lua_setfield(L, -2, "getDeviceAddress");
  lua_pop(L, 1);

  luaL_newmetatable(L, RaytracingPipeline::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRaytracingPipelineAddShader);
  lua_setfield(L, -2, "addShader");
  luaPushcfunction2(L, lRaytracingPipelineAddHitGroup);
  lua_setfield(L, -2, "addHitGroup");
  luaPushcfunction2(L, lRaytracingPipelineSetMaxPayloadSize);
  lua_setfield(L, -2, "setMaxPayloadSize");
  luaPushcfunction2(L, lRaytracingPipelineSetMaxAttributeSize);
  lua_setfield(L, -2, "setMaxAttributeSize");
  luaPushcfunction2(L, lRaytracingPipelineSetMaxRecursionDepth);
  lua_setfield(L, -2, "setMaxRecursionDepth");
  luaPushcfunction2(L, lRaytracingPipelineBuild);
  lua_setfield(L, -2, "build");
  luaPushcfunction2(L, lRaytracingPipelineIsReady);
  lua_setfield(L, -2, "isReady");
  luaPushcfunction2(L, lRaytracingPipelineGetShaderGroupHandleSize);
  lua_setfield(L, -2, "getShaderGroupHandleSize");
  luaPushcfunction2(L, lRaytracingPipelineGetShaderGroupHandles);
  lua_setfield(L, -2, "getShaderGroupHandles");
  lua_pop(L, 1);

  pushSnNamed(L, "Raytracing");
  luaPushcfunction2(L, lRaytracingIsSupported);
  lua_setfield(L, -2, "isDeviceSupported");
  luaPushcfunction2(L, lRaytracingIsSupported);
  lua_setfield(L, -2, "isSupported");
  luaPushcfunction2(L, lRaytracingIsRayQuerySupported);
  lua_setfield(L, -2, "isRayQuerySupported");
  luaPushcfunction2(L, lRaytracingCreateBottomLevel);
  lua_setfield(L, -2, "createBottomLevel");
  luaPushcfunction2(L, lRaytracingCreateTopLevel);
  lua_setfield(L, -2, "createTopLevel");
  luaPushcfunction2(L, lRaytracingDispatch);
  lua_setfield(L, -2, "dispatch");
  luaPushcfunction2(L, lRaytracingSetComputeAccelerationStructure);
  lua_setfield(L, -2, "setComputeAccelerationStructure");
  luaPushcfunction2(L, lRaytracingResetComputeAccelerationStructure);
  lua_setfield(L, -2, "resetComputeAccelerationStructure");
  luaPushcfunction2(L, lRaytracingResetAllComputeAccelerationStructures);
  lua_setfield(L, -2, "resetAllComputeAccelerationStructures");
  luaPushcfunction2(L, lRaytracingSetGraphicsAccelerationStructure);
  lua_setfield(L, -2, "setGraphicsAccelerationStructure");
  luaPushcfunction2(L, lRaytracingResetGraphicsAccelerationStructure);
  lua_setfield(L, -2, "resetGraphicsAccelerationStructure");
  luaPushcfunction2(L, lRaytracingResetAllGraphicsAccelerationStructures);
  lua_setfield(L, -2, "resetAllGraphicsAccelerationStructures");

  lua_newtable(L);
  luaPushcfunction2(L, lRaytracingPipelineNew);
  lua_setfield(L, -2, "new");
  lua_setfield(L, -2, "Pipeline");

  lua_newtable(L);
  lua_pushinteger(L, static_cast<lua_Integer>(
                         static_cast<UInt32>(gpu::RayTracingBuildFlags::None)));
  lua_setfield(L, -2, "None");
  lua_pushinteger(L, static_cast<lua_Integer>(static_cast<UInt32>(
                         gpu::RayTracingBuildFlags::AllowUpdate)));
  lua_setfield(L, -2, "AllowUpdate");
  lua_pushinteger(L, static_cast<lua_Integer>(static_cast<UInt32>(
                         gpu::RayTracingBuildFlags::PreferFastTrace)));
  lua_setfield(L, -2, "PreferFastTrace");
  lua_pushinteger(L, static_cast<lua_Integer>(static_cast<UInt32>(
                         gpu::RayTracingBuildFlags::PreferFastBuild)));
  lua_setfield(L, -2, "PreferFastBuild");
  lua_pushinteger(L, static_cast<lua_Integer>(static_cast<UInt32>(
                         gpu::RayTracingBuildFlags::MinimizeMemory)));
  lua_setfield(L, -2, "MinimizeMemory");
  lua_setfield(L, -2, "BuildFlags");

  lua_pop(L, 1);
}
} // namespace sinen
