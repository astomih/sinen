#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/compute/compute_pipeline.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>


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
  luaPushcfunction2(L, udPtrGc<ComputePipeline>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lComputePipelineSetShader);
  lua_setfield(L, -2, "setShader");
  luaPushcfunction2(L, lComputePipelineBuild);
  lua_setfield(L, -2, "build");
  luaPushcfunction2(L, lComputePipelineSetThreadGroupSize);
  lua_setfield(L, -2, "setThreadGroupSize");
  lua_pop(L, 1);

  pushSnNamed(L, "ComputePipeline");
  luaPushcfunction2(L, lComputePipelineNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
