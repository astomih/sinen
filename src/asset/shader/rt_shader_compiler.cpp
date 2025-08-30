#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "rt_shader_compiler.hpp"
#include <core/io/asset_io.hpp>

#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif
namespace rsc {
void diagnoseIfNeeded(slang::IBlob *diagnosticsBlob) {
  if (diagnosticsBlob != nullptr) {
    std::cout << (const char *)diagnosticsBlob->getBufferPointer() << std::endl;
  }
}

std::vector<char> ShaderCompiler::compile(std::string_view sourcePath,
                                          Type type, Language lang) {
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

  Slang::ComPtr<ISession> session;
  globalSession->createSession(sessionDesc, session.writeRef());

  Slang::ComPtr<slang::IModule> slangModule;
  {
    std::string moduleName;
    {

      size_t dotPos = sourcePath.find_last_of('.');
      auto view = (dotPos == std::string::npos) ? sourcePath
                                                : sourcePath.substr(0, dotPos);
      moduleName = view.data();
    }
    auto source =
        sinen::AssetIO::OpenAsString(sinen::AssetType::Shader, sourcePath);
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slangModule = session->loadModuleFromSourceString(
        moduleName.c_str(), // Module name
        sinen::AssetIO::ConvertFilePath(sinen::AssetType::Shader,
                                        sourcePath)
            .c_str(),                // Module path
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
    std::string entryPointName;
    switch (type) {
    case Type::VERTEX:
      entryPointName = "VSMain";
      break;
    case Type::FRAGMENT:
      entryPointName = "FSMain";
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

  // spirvCode to std::vector<char>
  std::vector<char> spirvData(spirvCode->getBufferSize());
  std::memcpy(spirvData.data(), spirvCode->getBufferPointer(),
              spirvCode->getBufferSize());
  return spirvData;
#else
  std::cout << "SLANG is not enabled. Cannot compile shader." << std::endl;
  return {};
#endif
}
} // namespace rsc