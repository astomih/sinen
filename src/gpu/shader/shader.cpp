// internal
#include "shader.hpp"
#include <graphics/graphics.hpp>
#include <platform/io/asset_io.hpp>
#include <script/luaapi.hpp>

#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>

#include "core/allocator/global_allocator.hpp"
#include "shader_bundle.hpp"
#include "shader_compiler.hpp"

// external
#include <SDL3/SDL.h>

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
  case ShaderStage::Compute:
    return ShaderStage::Compute;
  }
  return ShaderStage::Vertex;
}

ShaderFormat formatFromPath(StringView path) {
  constexpr StringView wgslExt = ".wgsl";
  constexpr StringView dxbcExt = ".dxbc";
  constexpr StringView dxilExt = ".dxil";
  if (path.size() >= wgslExt.size() &&
      path.substr(path.size() - wgslExt.size()) == wgslExt) {
    return ShaderFormat::WGSL;
  }
  if (path.size() >= dxbcExt.size() &&
      path.substr(path.size() - dxbcExt.size()) == dxbcExt) {
    return ShaderFormat::DXBC;
  }
  if (path.size() >= dxilExt.size() &&
      path.substr(path.size() - dxilExt.size()) == dxilExt) {
    return ShaderFormat::DXIL;
  }
  return ShaderFormat::SPIRV;
}

static void scheduleOnPreDraw(std::function<void()> f) {
  Graphics::addPreDrawFunc(std::move(f));
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
  const auto preferredFormat =
      ShaderBundle::preferredFormatFor(Graphics::getDevice()->getBackendAPI());
  this->format = shaderFormat;
  this->stage = stage;
  this->code.clear();

  const TaskGroup group = LoadContext::current();
  group.add();

  const String path = vertex_shader.data();
  state->future = globalThreadPool().submit(
      [state, path, stage, numUniformData, shaderFormat, preferredFormat] {
        auto str = AssetIO::openAsString(path);
        if (ShaderBundle::isBundle(str)) {
          auto selected = ShaderBundle::select(str, stage, preferredFormat);
          if (!selected) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Shader bundle does not contain requested backend "
                         "format");
            state->valid = false;
            return;
          }
          state->spirv = std::move(selected->code);
          state->shaderFormat = selected->format;
          state->numUniformBuffers = selected->numUniformBuffers;
          state->numSamplers = selected->numSamplers;
          state->numStorageBuffers = selected->numStorageBuffers;
          state->numStorageTextures = selected->numStorageTextures;
          state->gpuStage = selected->stage;
        } else {
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
        }
      });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state] {
        if (!state->valid) {
          this->async.reset();
          return;
        }
        auto *allocator = GlobalAllocator::get();
        auto device = Graphics::getDevice();

        gpu::Shader::CreateInfo info{};
        info.allocator = allocator;
        info.size = state->spirv.size();
        info.data = state->spirv.data();
        info.entrypoint =
            ShaderBundle::entryPointFor(state->gpuStage, state->shaderFormat);
        info.format = state->shaderFormat;
        info.stage = state->gpuStage;
        info.numSamplers = state->numSamplers;
        info.numStorageBuffers = state->numStorageBuffers;
        info.numStorageTextures = state->numStorageTextures;
        info.numUniformBuffers = state->numUniformBuffers;

        *shader = device->createShader(info);
        this->format = state->shaderFormat;
        this->code = state->spirv;
        this->async.reset();
      },
      [this] { this->async.reset(); });
}

void Shader::compile(StringView name, ShaderStage stage, ShaderFormat format) {
  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  this->stage = stage;
  this->format = format;
  this->code.clear();

  ShaderCompiler compiler;
  ShaderCompiler::ReflectionData reflectionData{};
  auto compiledCode = compiler.compile(name, stage, format, reflectionData);
  if (format == ShaderFormat::WGSL &&
      (compiledCode.empty() || compiledCode.back() != '\0')) {
    compiledCode.push_back('\0');
  }
  this->code = std::move(compiledCode);
}

void Shader::compileAndLoad(StringView name, ShaderStage stage) {
  GPUBackendAPI backendAPI = Graphics::getDevice()->getBackendAPI();
  ShaderFormat format = ShaderBundle::preferredFormatFor(backendAPI);
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
    ShaderCompiler compiler;
    ShaderCompiler::ReflectionData reflectionData{};
    state->spirv = compiler.compile(str, stage, format, reflectionData);
    if (format == ShaderFormat::WGSL &&
        (state->spirv.empty() || state->spirv.back() != '\0')) {
      state->spirv.push_back('\0');
    }

    state->shaderFormat = format;
    state->numUniformBuffers = reflectionData.numUniformBuffers;
    state->numSamplers = (stage == ShaderStage::Fragment)
                             ? reflectionData.numCombinedSamplers
                             : 0u;
    state->gpuStage = stage;
  });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state] {
        if (!state->valid) {
          this->async.reset();
          return;
        }
        auto *allocator = GlobalAllocator::get();
        auto device = Graphics::getDevice();

        gpu::Shader::CreateInfo info{};
        info.allocator = allocator;
        info.size = state->spirv.size();
        info.data = state->spirv.data();
        info.entrypoint =
            ShaderBundle::entryPointFor(state->gpuStage, state->shaderFormat);
        info.format = state->shaderFormat;
        info.stage = state->gpuStage;
        info.numSamplers = state->numSamplers;
        info.numStorageBuffers = state->numStorageBuffers;
        info.numStorageTextures = state->numStorageTextures;
        info.numUniformBuffers = state->numUniformBuffers;

        *shader = device->createShader(info);
        this->format = state->shaderFormat;
        this->code = state->spirv;
        this->async.reset();
      },
      [this] { this->async.reset(); });
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

static int lShaderCompile(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 4));
  s->compile(StringView(name), stage, format);
  return 0;
}

static int lShaderGetCode(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  udNewOwned<Buffer>(L, s->getCode());
  return 1;
}

static int lShaderIsReady(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushboolean(L, s->isReady());
  return 1;
}

void registerShader(lua_State *L) {
  luaL_newmetatable(L, Shader::metaTableName());
  luaPushcfunction2(L, udPtrGc<Shader>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lShaderLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lShaderCompile);
  lua_setfield(L, -2, "compile");
  luaPushcfunction2(L, lShaderCompileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  luaPushcfunction2(L, lShaderGetCode);
  lua_setfield(L, -2, "getCode");
  luaPushcfunction2(L, lShaderIsReady);
  lua_setfield(L, -2, "isReady");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  luaPushcfunction2(L, lShaderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
