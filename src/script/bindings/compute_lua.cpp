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

void registerCompute(luau::table &sn) {
  auto compute = sn.get_or_create_table("Compute");
  compute.function<lComputeSetComputePipeline>("setComputePipeline");
  compute.function<lComputeResetComputePipeline>("resetComputePipeline");
  compute.raw_function<lComputeSetUniformBuffer>("setUniformBuffer");
  compute.raw_function<lComputeSetStorageBuffer>("setStorageBuffer");
  compute.raw_function<lComputeResetStorageBuffer>("resetStorageBuffer");
  compute.function<lComputeResetAllStorageBuffers>("resetAllStorageBuffers");
  compute.raw_function<lComputeDispatch>("dispatch");
}
} // namespace sinen
