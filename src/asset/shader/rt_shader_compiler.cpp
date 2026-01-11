#include <array>
#include <cstdlib>
#include <iostream>

#include "rt_shader_compiler.hpp"

#include <platform/io/file.hpp>

#include <core/logger/logger.hpp>
#include <platform/io/asset_io.hpp>

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
  data.numUniformBuffers = 0;
  data.numCombinedSamplers = 0;

  if (!programLayout)
    return data;

  {

    slang::TypeLayoutReflection *typeLayout =
        programLayout->getGlobalParamsTypeLayout();
    const int rangeCount = typeLayout->getBindingRangeCount();

    int numUniformBuffers = 0;
    for (int i = 0; i < rangeCount; ++i) {
      auto rangeType = typeLayout->getBindingRangeType(i);
      if (rangeType == slang::BindingType::CombinedTextureSampler) {
        data.numCombinedSamplers++;
      }
      if (rangeType == slang::BindingType::ConstantBuffer) {
        data.numUniformBuffers++;
      }
    }
  }

  // Slang::ComPtr<ISlangBlob> blob;
  // programLayout->toJson(blob.writeRef());
  // if (blob) {
  //   File file;
  //   file.open("slang.json", File::Mode::w);
  //   file.write(blob->getBufferPointer(), blob->getBufferSize(), 1);
  //   file.close();
  // }
  return data;
}

Array<char> ShaderCompiler::compile(StringView sourcePath, ShaderStage stage,
                                    Language lang,
                                    ReflectionData &reflectionData) {
#if SINEN_USE_SLANG
  using namespace slang;

  Slang::ComPtr<IGlobalSession> globalSession;
  SlangGlobalSessionDesc desc = {};
  createGlobalSession(&desc, globalSession.writeRef());

  SessionDesc sessionDesc = {};

  std::array<TargetDesc, 1> targetDesc = {};
  targetDesc[0].format = SLANG_SPIRV;
  targetDesc[0].profile = globalSession->findProfile("spirv_1_3");
  sessionDesc.targets = targetDesc.data();
  sessionDesc.targetCount = targetDesc.size();
  std::array<slang::CompilerOptionEntry, 1> options = {
      {slang::CompilerOptionName::EmitSpirvDirectly,
       {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}}};
  sessionDesc.compilerOptionEntries = options.data();
  sessionDesc.compilerOptionEntryCount = options.size();
  sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

  Slang::ComPtr<ISession> session;
  globalSession->createSession(sessionDesc, session.writeRef());

  Slang::ComPtr<slang::IModule> slangModule;
  {
    String moduleName;
    {

      size_t dotPos = sourcePath.find_last_of('.');
      auto view =
          (dotPos == String::npos) ? sourcePath : sourcePath.substr(0, dotPos);
      moduleName = view.data();
    }
    auto source = sinen::AssetIO::openAsString(sourcePath);
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slangModule = session->loadModuleFromSourceString(
        moduleName.c_str(),                              // Module name
        sinen::AssetIO::getFilePath(sourcePath).c_str(), // Module path
        source.data(),                                   // Shader source code
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
  Slang::ComPtr<slang::IBlob> spirvCode;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result = linkedProgram->getEntryPointCode(
        0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) {
      std::cout << "Error getting SPIR-V code: " << result << std::endl;
      return {};
    }
  }

  // TODO: Get reflection (num bindings, etc.)
  reflectionData = getReflectionData(linkedProgram);
  Array<char> spirvData(spirvCode->getBufferSize());
  std::memcpy(spirvData.data(), spirvCode->getBufferPointer(),
              spirvCode->getBufferSize());
  return spirvData;
#else
  std::cout << "SLANG is not enabled. Cannot compile shader." << std::endl;
  return {};
#endif
}
} // namespace sinen