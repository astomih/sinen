// internal
#include "shader.hpp"
#include <graphics/graphics.hpp>
#include <platform/io/asset_reader.hpp>

#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>

#include "core/allocator/global_allocator.hpp"
#include "shader_bundle.hpp"
#ifdef SINEN_MODULE_SHADER_COMPILER
#include <shader_compiler/shader_compiler.hpp>
#endif

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
  case ShaderStage::RayGeneration:
    return ShaderStage::RayGeneration;
  case ShaderStage::AnyHit:
    return ShaderStage::AnyHit;
  case ShaderStage::ClosestHit:
    return ShaderStage::ClosestHit;
  case ShaderStage::Miss:
    return ShaderStage::Miss;
  case ShaderStage::Intersection:
    return ShaderStage::Intersection;
  case ShaderStage::Callable:
    return ShaderStage::Callable;
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

#ifdef SINEN_MODULE_SHADER_COMPILER
static Array<Shader::ResourceBinding>
copyResourceBindings(const Array<ShaderCompiler::ResourceBinding> &bindings) {
  Array<Shader::ResourceBinding> result;
  result.reserve(bindings.size());
  for (const auto &binding : bindings) {
    result.push_back({binding.name, binding.slot});
  }
  return result;
}
#endif
} // namespace

Shader::Shader() {
  shader = makePtr<Ptr<gpu::Shader>>();
  resourceBindings = makePtr<ResourceBindings>();
}
Shader::Shader(const Ptr<gpu::Shader> &raw) {
  shader = makePtr<Ptr<gpu::Shader>>();
  resourceBindings = makePtr<ResourceBindings>();
  *shader = raw;
  if (raw != nullptr) {
    const auto &createInfo = raw->getCreateInfo();
    format = createInfo.format;
    stage = toShaderStage(createInfo.stage);
    numSamplers = createInfo.numSamplers;
    numStorageBuffers = createInfo.numStorageBuffers;
    numStorageTextures = createInfo.numStorageTextures;
    numUniformBuffers = createInfo.numUniformBuffers;
  }
}
Shader::~Shader() {
  shader.reset();
  async.reset();
}

void Shader::load(StringView vertex_shader, ShaderStage stage) {
  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  resourceBindings = makePtr<ResourceBindings>();
  const Ptr<ResourceBindings> bindings = resourceBindings;
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;
  state->debugName = "Shader::load(" + String(vertex_shader) + ")";
  const auto shaderFormat = formatFromPath(vertex_shader);
  const auto preferredFormat =
      ShaderBundle::preferredFormatFor(Graphics::getDevice()->getBackendAPI());
  this->format = shaderFormat;
  this->stage = stage;
  this->code.clear();
  this->numSamplers = 0;
  this->numStorageBuffers = 0;
  this->numStorageTextures = 0;
  this->numUniformBuffers = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  const String path = vertex_shader.data();
  state->future = globalThreadPool().submit(
      [state, path, stage, shaderFormat, preferredFormat] {
        auto str = AssetReader::readAsString(path);
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
          state->numUniformBuffers = 1;
          state->numSamplers = (stage == ShaderStage::Fragment) ? 1u : 0u;
          state->gpuStage = stage;
        }
      });
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state, bindings] {
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
        this->numSamplers = state->numSamplers;
        this->numStorageBuffers = state->numStorageBuffers;
        this->numStorageTextures = state->numStorageTextures;
        this->numUniformBuffers = state->numUniformBuffers;
        bindings->uniformBuffers = state->uniformBuffers;
        bindings->textures = state->textures;
        this->async.reset();
      },
      [this] { this->async.reset(); });
}

void Shader::compile(StringView name, ShaderStage stage, ShaderFormat format) {
  this->shader.reset();
  shader = makePtr<Ptr<gpu::Shader>>();
  resourceBindings = makePtr<ResourceBindings>();
  this->stage = stage;
  this->format = format;
  this->code.clear();
  this->numSamplers = 0;
  this->numStorageBuffers = 0;
  this->numStorageTextures = 0;
  this->numUniformBuffers = 0;

#ifdef SINEN_MODULE_SHADER_COMPILER
  ShaderCompiler compiler;
  ShaderCompiler::ReflectionData reflectionData{};
  auto compiledCode = compiler.compile(name, stage, format, reflectionData);
  if (format == ShaderFormat::WGSL &&
      (compiledCode.empty() || compiledCode.back() != '\0')) {
    compiledCode.push_back('\0');
  }
  this->code = std::move(compiledCode);
  this->numSamplers = reflectionData.numCombinedSamplers;
  this->numStorageBuffers = reflectionData.numStorageBuffers;
  this->numStorageTextures = reflectionData.numStorageTextures;
  this->numUniformBuffers = reflectionData.numUniformBuffers;
  resourceBindings->uniformBuffers =
      copyResourceBindings(reflectionData.uniformBuffers);
  resourceBindings->textures = copyResourceBindings(reflectionData.textures);
#else
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
               "ShaderCompiler module is disabled. Cannot compile shader.");
#endif
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
  resourceBindings = makePtr<ResourceBindings>();
  const Ptr<ResourceBindings> bindings = resourceBindings;
  this->async = makePtr<AsyncState>();
  const Ptr<AsyncState> state = this->async;
  state->debugName = "Shader::compileAndLoad(" + String(name) + ")";
  this->stage = stage;
  this->code.clear();
  this->numSamplers = 0;
  this->numStorageBuffers = 0;
  this->numStorageTextures = 0;
  this->numUniformBuffers = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

#ifdef SINEN_MODULE_SHADER_COMPILER
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
    state->numSamplers = reflectionData.numCombinedSamplers;
    state->numStorageBuffers = reflectionData.numStorageBuffers;
    state->numStorageTextures = reflectionData.numStorageTextures;
    state->uniformBuffers = copyResourceBindings(reflectionData.uniformBuffers);
    state->textures = copyResourceBindings(reflectionData.textures);
    state->gpuStage = stage;
  });
#else
  state->future = globalThreadPool().submit([state] {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "ShaderCompiler module is disabled. Cannot compile shader.");
    state->valid = false;
  });
#endif
  scheduleFuturePoll(
      state, group, scheduleOnPreDraw,
      [this, state, bindings] {
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
        this->numSamplers = state->numSamplers;
        this->numStorageBuffers = state->numStorageBuffers;
        this->numStorageTextures = state->numStorageTextures;
        this->numUniformBuffers = state->numUniformBuffers;
        bindings->uniformBuffers = state->uniformBuffers;
        bindings->textures = state->textures;
        this->async.reset();
      },
      [this] { this->async.reset(); });
}
Ptr<gpu::Shader> Shader::getRaw() { return *shader; }
ShaderFormat Shader::getFormat() const { return format; }
ShaderStage Shader::getStage() const { return stage; }
uint32_t Shader::getNumSamplers() const { return numSamplers; }
uint32_t Shader::getNumStorageBuffers() const { return numStorageBuffers; }
uint32_t Shader::getNumStorageTextures() const { return numStorageTextures; }
uint32_t Shader::getNumUniformBuffers() const { return numUniformBuffers; }
bool Shader::findUniformBufferSlot(StringView name, uint32_t &slot) const {
  if (!resourceBindings) {
    return false;
  }
  for (const auto &binding : resourceBindings->uniformBuffers) {
    if (binding.name == name) {
      slot = binding.slot;
      return true;
    }
  }
  return false;
}
bool Shader::findTextureSlot(StringView name, uint32_t &slot) const {
  if (!resourceBindings) {
    return false;
  }
  for (const auto &binding : resourceBindings->textures) {
    if (binding.name == name) {
      slot = binding.slot;
      return true;
    }
  }
  return false;
}
Buffer Shader::getCode() const {
  if (code.empty()) {
    return Buffer(BufferType::Binary, Ptr<void>(), 0);
  }
  auto ret = makeBuffer(code.size(), BufferType::Binary);
  std::memcpy(ret.data(), code.data(), code.size());
  return ret;
}

} // namespace sinen
