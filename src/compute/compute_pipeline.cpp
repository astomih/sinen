#include "compute_pipeline.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

#include <functional>
#include <memory>

namespace sinen {
void ComputePipeline::setShader(const Shader &shader) {
  this->computeShader = shader;
}

void ComputePipeline::setThreadGroupSize(UInt32 x, UInt32 y, UInt32 z) {
  threadCountX = x;
  threadCountY = y;
  threadCountZ = z;
}

void ComputePipeline::build() {
  const TaskGroup group = LoadContext::current();
  group.add();

  const bool inSetup = (LoadContext::currentPtr() != nullptr);

  auto buildOrRetry = std::make_shared<std::function<void()>>();
  *buildOrRetry = [this, buildOrRetry, group]() {
    if (this->pipeline) {
      group.done();
      return;
    }

    if (!this->computeShader.getRaw()) {
      Graphics::addPreDrawFunc(*buildOrRetry);
      return;
    }

    gpu::ComputePipeline::CreateInfo pipelineInfo{};
    pipelineInfo.allocator = GlobalAllocator::get();
    pipelineInfo.computeShader = this->computeShader.getRaw();
    pipelineInfo.threadCountX = threadCountX;
    pipelineInfo.threadCountY = threadCountY;
    pipelineInfo.threadCountZ = threadCountZ;
    this->pipeline = Graphics::getDevice()->createComputePipeline(pipelineInfo);
    group.done();
  };

  if (!inSetup && this->computeShader.getRaw()) {
    (*buildOrRetry)();
    return;
  }

  Graphics::addPreDrawFunc(*buildOrRetry);
}

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
