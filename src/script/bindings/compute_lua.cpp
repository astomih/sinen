#include "luaapi.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/logger/log.hpp>
#include <gpu/compute/compute.hpp>
#include <gpu/compute/compute_buffer.hpp>
#include <gpu/compute/compute_pipeline.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
static auto lComputeSetComputePipeline(Ptr<ComputePipeline> &pipeline) {
  Compute::setComputePipeline(*pipeline);
  return;
}

static auto lComputeResetComputePipeline() {
  Compute::resetComputePipeline();
  return;
}

static int lComputeSetUniformBuffer(lua_State *L) {
  auto &buffer = udValue<Buffer>(L, 2);
  if (lua_type(L, 1) == LUA_TSTRING) {
    const char *name = luaL_checkstring(L, 1);
    Compute::setUniformBuffer(StringView(name), buffer);
  } else {
    auto slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
    Compute::setUniformBuffer(slot, buffer);
  }
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

static auto lComputeResetAllStorageBuffers() {
  Compute::resetAllStorageBuffers();
  return;
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
  luau::table(L, -1).function("setComputePipeline", lComputeSetComputePipeline);
  luau::table(L, -1).function("resetComputePipeline",
                              lComputeResetComputePipeline);
  luau::table(L, -1).raw_function("setUniformBuffer", lComputeSetUniformBuffer);
  luau::table(L, -1).raw_function("setStorageBuffer", lComputeSetStorageBuffer);
  luau::table(L, -1).raw_function("resetStorageBuffer",
                                  lComputeResetStorageBuffer);
  luau::table(L, -1).function("resetAllStorageBuffers",
                              lComputeResetAllStorageBuffers);
  luau::table(L, -1).raw_function("dispatch", lComputeDispatch);
  lua_pop(L, 1);
}
} // namespace sinen
