// internal
#include <graphics/graphics.hpp>
#include <graphics/shader/shader.hpp>
#include <platform/io/asset_io.hpp>
#include <script/luaapi.hpp>

#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>

#include "core/allocator/global_allocator.hpp"
#include "rt_shader_compiler.hpp"

// external
#include <SDL3/SDL.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <memory>

namespace sinen {
Shader::Shader() { shader = makePtr<Ptr<gpu::Shader>>(); }
Shader::Shader(const Ptr<gpu::Shader> &raw) {
  shader = makePtr<Ptr<gpu::Shader>>();
  *shader = raw;
}
void Shader::load(StringView vertex_shader, ShaderStage stage,
                  int numUniformData) {
  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;

  const TaskGroup group = LoadContext::current();
  group.add();

  const String path = vertex_shader.data();
  state->future =
      globalThreadPool().submit([state, path, stage, numUniformData] {
        auto str = AssetIO::openAsString(path);
        state->spirv.resize(str.size());
        if (!str.empty()) {
          std::memcpy(state->spirv.data(), str.data(), str.size());
        }

        state->numUniformBuffers = static_cast<uint32_t>(numUniformData + 1);
        state->numSamplers = (stage == ShaderStage::Fragment) ? 1u : 0u;

        switch (stage) {
        case ShaderStage::Vertex:
          state->gpuStage = gpu::ShaderStage::Vertex;
          break;
        case ShaderStage::Fragment:
          state->gpuStage = gpu::ShaderStage::Fragment;
          break;
        case ShaderStage::Compute:
          state->gpuStage = gpu::ShaderStage::Vertex; // TODO
          break;
        }
      });

  auto pollAndCreate = std::make_shared<std::function<void()>>();
  *pollAndCreate = [this, pollAndCreate, state, group]() {
    if (!state->future.valid()) {
      group.done();
      return;
    }
    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndCreate);
      return;
    }

    state->future.get();

    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo info{};
    info.allocator = allocator;
    info.size = state->spirv.size();
    info.data = state->spirv.data();
    info.entrypoint = "main";
    info.format = gpu::ShaderFormat::SPIRV;
    info.stage = state->gpuStage;
    info.numSamplers = state->numSamplers;
    info.numStorageBuffers = 0;
    info.numStorageTextures = 0;
    info.numUniformBuffers = state->numUniformBuffers;

    *shader = device->createShader(info);
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndCreate);
}
void Shader::compileAndLoad(StringView name, ShaderStage stage) {

  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;

  const TaskGroup group = LoadContext::current();
  group.add();

  const String str = name.data();
  state->future = globalThreadPool().submit([state, str, stage] {
    // TODO: add support for other languages
    ShaderCompiler compiler;
    ShaderCompiler::ReflectionData reflectionData{};
    state->spirv = compiler.compile(str, stage, ShaderCompiler::Language::SPIRV,
                                    reflectionData);

    state->numUniformBuffers = reflectionData.numUniformBuffers;
    state->numSamplers = (stage == ShaderStage::Fragment)
                             ? reflectionData.numCombinedSamplers
                             : 0u;

    switch (stage) {
    case ShaderStage::Vertex:
      state->gpuStage = gpu::ShaderStage::Vertex;
      break;
    case ShaderStage::Fragment:
      state->gpuStage = gpu::ShaderStage::Fragment;
      break;
    case ShaderStage::Compute:
      state->gpuStage = gpu::ShaderStage::Vertex; // TODO
      break;
    }
  });

  auto pollAndCreate = std::make_shared<std::function<void()>>();
  *pollAndCreate = [this, pollAndCreate, state, group]() {
    if (!state->future.valid()) {
      group.done();
      return;
    }
    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndCreate);
      return;
    }

    state->future.get();

    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::Shader::CreateInfo info{};
    info.allocator = allocator;
    info.size = state->spirv.size();
    info.data = state->spirv.data();
    info.entrypoint = "main";
    info.format = gpu::ShaderFormat::SPIRV;
    info.stage = state->gpuStage;
    info.numSamplers = state->numSamplers;
    info.numStorageBuffers = 0;
    info.numStorageTextures = 0;
    info.numUniformBuffers = state->numUniformBuffers;

    {
      *shader = device->createShader(info);
    }
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndCreate);
}
Ptr<gpu::Shader> Shader::getRaw() { return *shader; }

static int lShaderNew(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>());
  return 1;
}
static int lShaderLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  int numUniformData = static_cast<int>(luaL_checkinteger(L, 4));
  s->load(StringView(name), stage, numUniformData);
  return 0;
}
static int lShaderCompileAndLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  s->compileAndLoad(StringView(name), stage);
  return 0;
}
void registerShader(lua_State *L) {
  luaL_newmetatable(L, Shader::metaTableName());
  luaPushcfunction2(L, udPtrGc<Shader>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lShaderLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lShaderCompileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  luaPushcfunction2(L, lShaderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
