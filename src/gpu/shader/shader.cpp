// internal
#include "shader.hpp"
#include <graphics/graphics.hpp>
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
namespace {

ShaderStage toShaderStage(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vertex:
    return ShaderStage::Vertex;
  case ShaderStage::Fragment:
    return ShaderStage::Fragment;
  }
  return ShaderStage::Vertex;
}

const char *entryPointFor(ShaderStage stage, ShaderFormat format) {
  if (format == ShaderFormat::WGSL) {
    switch (stage) {
    case ShaderStage::Vertex:
      return "VSMain";
    case ShaderStage::Fragment:
      return "FSMain";
    case ShaderStage::Compute:
      return "CSMain";
    }
  }
  return "main";
}

ShaderFormat formatFromPath(StringView path) {
  constexpr StringView wgslExt = ".wgsl";
  if (path.size() >= wgslExt.size() &&
      path.substr(path.size() - wgslExt.size()) == wgslExt) {
    return ShaderFormat::WGSL;
  }
  return ShaderFormat::SPIRV;
}
} // namespace

Shader::Shader() { shader = makePtr<Ptr<gpu::Shader>>(); }
Shader::Shader(const Ptr<gpu::Shader> &raw) {
  shader = makePtr<Ptr<gpu::Shader>>();
  *shader = raw;
  if (raw != nullptr) {
    const auto &createInfo = raw->getCreateInfo();
    format = createInfo.format;
    stage = toShaderStage(createInfo.stage);
  }
}
void Shader::load(StringView vertex_shader, ShaderStage stage,
                  int numUniformData) {
  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;
  const auto shaderFormat = formatFromPath(vertex_shader);
  this->format = shaderFormat;
  this->stage = stage;
  this->code.clear();

  const TaskGroup group = LoadContext::current();
  group.add();

  const String path = vertex_shader.data();
  state->future = globalThreadPool().submit(
      [state, path, stage, numUniformData, shaderFormat] {
        auto str = AssetIO::openAsString(path);
        if (shaderFormat == ShaderFormat::WGSL &&
            (str.empty() || str.back() != '\0')) {
          str.push_back('\0');
        }
        state->spirv.resize(str.size());
        if (!str.empty()) {
          std::memcpy(state->spirv.data(), str.data(), str.size());
        }

        state->shaderFormat = shaderFormat;
        state->numUniformBuffers = static_cast<uint32_t>(numUniformData + 1);
        state->numSamplers = (stage == ShaderStage::Fragment) ? 1u : 0u;
        state->gpuStage = stage;
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
    info.entrypoint = entryPointFor(this->stage, state->shaderFormat);
    info.format = state->shaderFormat;
    info.stage = state->gpuStage;
    info.numSamplers = state->numSamplers;
    info.numStorageBuffers = 0;
    info.numStorageTextures = 0;
    info.numUniformBuffers = state->numUniformBuffers;

    *shader = device->createShader(info);
    this->format = state->shaderFormat;
    this->code = state->spirv;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndCreate);
}
void Shader::compileAndLoad(StringView name, ShaderStage stage) {
  GPUBackendAPI backendAPI = Graphics::getDevice()->getBackendAPI();
  ShaderFormat format;
  switch (backendAPI) {
  case GPUBackendAPI::WebGPU:
    format = ShaderFormat::WGSL;
    break;
  default:
    format = ShaderFormat::SPIRV;
  }
  compileAndLoad(name, stage, format);
}
void Shader::compileAndLoad(StringView name, ShaderStage stage,
                            ShaderFormat format) {

  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;
  this->stage = stage;
  this->code.clear();

  const TaskGroup group = LoadContext::current();
  group.add();

  const String str = name.data();
  state->future = globalThreadPool().submit([state, str, stage, format] {
    auto device = Graphics::getDevice();
    ShaderCompiler compiler;
    ShaderCompiler::ReflectionData reflectionData{};
    state->spirv = compiler.compile(str, stage, format, reflectionData);
    if (format == ShaderFormat::WGSL &&
        (state->spirv.empty() || state->spirv.back() != '\0')) {
      state->spirv.push_back('\0');
    }

    state->shaderFormat = (format == ShaderFormat::WGSL) ? ShaderFormat::WGSL
                                                         : ShaderFormat::SPIRV;
    state->numUniformBuffers = reflectionData.numUniformBuffers;
    state->numSamplers = (stage == ShaderStage::Fragment)
                             ? reflectionData.numCombinedSamplers
                             : 0u;
    state->gpuStage = stage;
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
    info.entrypoint = entryPointFor(this->stage, state->shaderFormat);
    info.format = state->shaderFormat;
    info.stage = state->gpuStage;
    info.numSamplers = state->numSamplers;
    info.numStorageBuffers = 0;
    info.numStorageTextures = 0;
    info.numUniformBuffers = state->numUniformBuffers;

    {
      *shader = device->createShader(info);
    }
    this->format = state->shaderFormat;
    this->code = state->spirv;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndCreate);
}
Ptr<gpu::Shader> Shader::getRaw() { return *shader; }
ShaderFormat Shader::getFormat() const { return format; }
ShaderStage Shader::getStage() const { return stage; }
Buffer Shader::getCode() const {
  if (code.empty()) {
    return Buffer(BufferType::Binary, Ptr<void>(), 0);
  }
  auto ret = makeBuffer(code.size(), BufferType::Binary);
  std::memcpy(ret.data(), code.data(), code.size());
  return ret;
}

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

  if (lua_gettop(L) >= 4) {
    auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 4));
    s->compileAndLoad(StringView(name), stage, format);
  } else {
    s->compileAndLoad(StringView(name), stage);
  }
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
