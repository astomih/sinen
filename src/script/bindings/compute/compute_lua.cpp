#include <core/allocator/global_allocator.hpp>
#include <core/logger/log.hpp>
#include <gpu/compute/compute.hpp>
#include <gpu/compute/compute_buffer.hpp>
#include <gpu/compute/compute_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>


namespace sinen {
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
