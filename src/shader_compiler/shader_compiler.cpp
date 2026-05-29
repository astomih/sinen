#include <array>
#include <cstdlib>
#include <iostream>

#include <shader_compiler/shader_compiler.hpp>

#include <platform/io/asset_reader.hpp>

#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif
namespace sinen {
void diagnoseIfNeeded(slang::IBlob *diagnosticsBlob) {
  if (diagnosticsBlob != nullptr) {
    std::cout << (const char *)diagnosticsBlob->getBufferPointer() << std::endl;
  }
}
ShaderCompiler::ReflectionData
getReflectionData(slang::IComponentType *program) {
  auto *programLayout = program->getLayout();
  ShaderCompiler::ReflectionData data;

  if (!programLayout)
    return data;

  {

    slang::TypeLayoutReflection *typeLayout =
        programLayout->getGlobalParamsTypeLayout();
    const int rangeCount = typeLayout->getBindingRangeCount();

    for (int i = 0; i < rangeCount; ++i) {
      auto rangeType = typeLayout->getBindingRangeType(i);
      if (rangeType == slang::BindingType::CombinedTextureSampler ||
          rangeType == slang::BindingType::Sampler) {
        data.numCombinedSamplers++;
      }
      if (rangeType == slang::BindingType::ConstantBuffer) {
        data.numUniformBuffers++;
      }
      if (rangeType == slang::BindingType::MutableRawBuffer ||
          rangeType == slang::BindingType::MutableTypedBuffer ||
          rangeType == slang::BindingType::RawBuffer ||
          rangeType == slang::BindingType::TypedBuffer) {
        data.numStorageBuffers++;
      }
      if (rangeType == slang::BindingType::MutableTexture) {
        data.numStorageTextures++;
      }
    }
  }

  return data;
}

Array<char> ShaderCompiler::compileSource(StringView moduleName,
                                          StringView modulePath,
                                          StringView source, ShaderStage stage,
                                          ShaderFormat format,
                                          ReflectionData &reflectionData) {
#if SINEN_USE_SLANG
  using namespace slang;

  Slang::ComPtr<IGlobalSession> globalSession;
  SlangGlobalSessionDesc desc = {};
  createGlobalSession(&desc, globalSession.writeRef());

  SessionDesc sessionDesc = {};

  std::array<TargetDesc, 1> targetDesc = {};
  bool emitSpirvDirectly = false;
  switch (format) {
  case ShaderFormat::SPIRV:
    targetDesc[0].format = SLANG_SPIRV;
    targetDesc[0].profile = globalSession->findProfile("spirv_1_4");
    emitSpirvDirectly = true;
    break;
  case ShaderFormat::WGSL:
    targetDesc[0].format = SLANG_WGSL;
    targetDesc[0].profile = globalSession->findProfile("wgsl_1_0");
    break;
  case ShaderFormat::DXBC:
    targetDesc[0].format = SLANG_DXBC;
    targetDesc[0].profile = globalSession->findProfile("sm_5_1");
    break;
  case ShaderFormat::DXIL:
    targetDesc[0].format = SLANG_DXIL;
    targetDesc[0].profile = globalSession->findProfile(
        stage == ShaderStage::RayGeneration || stage == ShaderStage::AnyHit ||
                stage == ShaderStage::ClosestHit ||
                stage == ShaderStage::Miss ||
                stage == ShaderStage::Intersection ||
                stage == ShaderStage::Callable
            ? "lib_6_3"
            : "sm_6_5");
    break;
  }
  sessionDesc.targets = targetDesc.data();
  sessionDesc.targetCount = targetDesc.size();
  std::array<slang::PreprocessorMacroDesc, 1> macros = {};
  if (format == ShaderFormat::WGSL) {
    macros[0] = {"SINEN_TARGET_WEBGPU", "1"};
    sessionDesc.preprocessorMacros = macros.data();
    sessionDesc.preprocessorMacroCount = macros.size();
  }
  std::array<slang::CompilerOptionEntry, 1> options = {};
  if (emitSpirvDirectly) {
    options[0] = {
        slang::CompilerOptionName::EmitSpirvDirectly,
        {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}};
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();
  }
  sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

  Slang::ComPtr<ISession> session;
  globalSession->createSession(sessionDesc, session.writeRef());

  Slang::ComPtr<slang::IModule> slangModule;
  {
    String moduleNameString(moduleName.data(), moduleName.size());
    String modulePathString(modulePath.data(), modulePath.size());
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slangModule = session->loadModuleFromSourceString(
        moduleNameString.c_str(),    // Module name
        modulePathString.c_str(),    // Module path
        source.data(),               // Shader source code
        diagnosticsBlob.writeRef()); // Optional diagnostic container
    if (!slangModule) {
      diagnoseIfNeeded(diagnosticsBlob);
      return {};
    }
  }

  // 4. Query Entry Points
  Slang::ComPtr<slang::IEntryPoint> entryPoint;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    String entryPointName;
    switch (stage) {
    case ShaderStage::Vertex:
      entryPointName = "VSMain";
      break;
    case ShaderStage::Fragment:
      entryPointName = "FSMain";
      break;
    case ShaderStage::Compute:
      entryPointName = "CSMain";
      break;
    case ShaderStage::RayGeneration:
      entryPointName = "RayGenMain";
      break;
    case ShaderStage::AnyHit:
      entryPointName = "AnyHitMain";
      break;
    case ShaderStage::ClosestHit:
      entryPointName = "ClosestHitMain";
      break;
    case ShaderStage::Miss:
      entryPointName = "MissMain";
      break;
    case ShaderStage::Intersection:
      entryPointName = "IntersectionMain";
      break;
    case ShaderStage::Callable:
      entryPointName = "CallableMain";
      break;
    default: // Unsupported type
      std::cout << "Unsupported shader type" << std::endl;
      return {};
    }
    slangModule->findEntryPointByName(entryPointName.c_str(),
                                      entryPoint.writeRef());
    diagnoseIfNeeded(diagnosticsBlob);
    if (!entryPoint) {
      std::cout << "Error getting entry point" << std::endl;
      return {};
    }
  }

  // 5. Compose Modules + Entry Points
  std::array<slang::IComponentType *, 2> componentTypes = {slangModule,
                                                           entryPoint};

  Slang::ComPtr<slang::IComponentType> composedProgram;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = session->createCompositeComponentType(
        componentTypes.data(), componentTypes.size(),
        composedProgram.writeRef(), diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) {
      std::cout << "Error composing program: " << result << std::endl;
      return {};
    }
  }

  // 6. Link
  Slang::ComPtr<slang::IComponentType> linkedProgram;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = composedProgram->link(linkedProgram.writeRef(),
                                               diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) {
      std::cout << "Error linking program: " << result << std::endl;
      return {};
    }
  }

  // 7. Get Target Kernel Code
  Slang::ComPtr<slang::IBlob> compiledCode;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = linkedProgram->getEntryPointCode(
        0, 0, compiledCode.writeRef(), diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) {
      std::cout << "Error getting shader code: " << result << std::endl;
      return {};
    }
  }

  // TODO: Get reflection (num bindings, etc.)
  reflectionData = getReflectionData(linkedProgram);
  Array<char> shaderData(compiledCode->getBufferSize());
  std::memcpy(shaderData.data(), compiledCode->getBufferPointer(),
              compiledCode->getBufferSize());
  return shaderData;
#else
  std::cout << "SLANG is not enabled. Cannot compile shader." << std::endl;
  return {};
#endif
}

Array<char> ShaderCompiler::compile(StringView sourcePath, ShaderStage stage,
                                    ShaderFormat format,
                                    ReflectionData &reflectionData) {
  String moduleName;
  {
    size_t dotPos = sourcePath.find_last_of('.');
    auto view =
        (dotPos == String::npos) ? sourcePath : sourcePath.substr(0, dotPos);
    moduleName = String(view.data(), view.size());
  }
  auto source = sinen::AssetReader::openAsString(sourcePath);
  auto modulePath = sinen::AssetReader::getLoadPath(sourcePath);
  return compileSource(moduleName, modulePath, source, stage, format,
                       reflectionData);
}
} // namespace sinen
