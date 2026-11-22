#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/compute/compute_pipeline.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
static auto lComputePipelineNew() { return makePtr<ComputePipeline>(); }

static auto lComputePipelineSetShader(Ptr<ComputePipeline> &pipeline,
                                      Ptr<Shader> &shader) {
  pipeline->setShader(*shader);
  return;
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

void registerComputePipeline(luau::table &sn) {
  auto computePipeline =
      engineClass<Ptr<ComputePipeline>>(sn, ComputePipeline::metaTableName());

  computePipeline.method<lComputePipelineSetShader>("setShader");
  computePipeline.raw_method<lComputePipelineBuild>("build");
  computePipeline.raw_method<lComputePipelineSetThreadGroupSize>(
      "setThreadGroupSize");

  computePipeline.static_function<lComputePipelineNew>("new");
}
} // namespace sinen
