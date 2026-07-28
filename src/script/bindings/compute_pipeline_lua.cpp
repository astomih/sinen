#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/compute/compute_pipeline.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
static int lComputePipelineNew(lua_State *L) {
  udPushPtr<ComputePipeline>(L, makePtr<ComputePipeline>());
  return 1;
}

static int lComputePipelineSetShader(lua_State *L) {
  auto &pipeline = udPtr<ComputePipeline>(L, 1);
  auto &shader = udPtr<Shader>(L, 2);
  pipeline->setShader(*shader);
  return 0;
}

static int lComputePipelineBuild(lua_State *L) {
  udPtr<ComputePipeline>(L, 1)->build();
  return 0;
}

static int lComputePipelineSetThreadGroupSize(lua_State *L) {
  auto &pipeline = udPtr<ComputePipeline>(L, 1);
  auto x = static_cast<UInt32>(luaL_checkinteger(L, 2));
  auto y = static_cast<UInt32>(luaL_optinteger(L, 3, 1));
  auto z = static_cast<UInt32>(luaL_optinteger(L, 4, 1));
  pipeline->setThreadGroupSize(x, y, z);
  return 0;
}

void registerComputePipeline(lua_State *L) {
  luaL_newmetatable(L, ComputePipeline::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "setShader", lComputePipelineSetShader);
  Binding::registerFunction(L, "build", lComputePipelineBuild);
  Binding::registerFunction(L, "setThreadGroupSize",
                            lComputePipelineSetThreadGroupSize);
  lua_pop(L, 1);

  pushSnNamed(L, "ComputePipeline");
  Binding::registerFunction(L, "new", lComputePipelineNew);
  lua_pop(L, 1);
}
} // namespace sinen
