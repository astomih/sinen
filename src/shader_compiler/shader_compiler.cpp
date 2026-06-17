#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>

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
namespace {
void diagnoseIfNeeded(slang::IBlob *diagnosticsBlob) {
  if (diagnosticsBlob != nullptr) {
    std::cout << (const char *)diagnosticsBlob->getBufferPointer() << std::endl;
  }
}

bool isKnownNonNegative(SlangInt value) { return value >= 0; }

bool containsId(const Array<uint32_t> &ids, uint32_t id) {
  for (uint32_t existing : ids) {
    if (existing == id) {
      return true;
    }
  }
  return false;
}

void pushUniqueId(Array<uint32_t> &ids, uint32_t id) {
  if (!containsId(ids, id)) {
    ids.push_back(id);
  }
}

struct BindingDecoration {
  uint32_t targetId;
  uint32_t binding;
};

std::optional<uint32_t>
findBindingDecoration(const Array<BindingDecoration> &decorations,
                      uint32_t targetId) {
  for (const BindingDecoration &decoration : decorations) {
    if (decoration.targetId == targetId) {
      return decoration.binding;
    }
  }
  return std::nullopt;
}

uint32_t readSpirvWord(const void *data, size_t wordIndex) {
  uint32_t value = 0;
  std::memcpy(&value, static_cast<const char *>(data) + wordIndex * 4,
              sizeof(value));
  return value;
}

std::optional<uint32_t> countSpirvUniformBufferSlots(const void *data,
                                                     size_t size) {
  if (!data || size < 20 || (size % 4) != 0) {
    return std::nullopt;
  }

  constexpr uint32_t opDecorate = 71;
  constexpr uint32_t opVariable = 59;
  constexpr uint32_t decorationBinding = 33;
  constexpr uint32_t storageClassUniform = 2;

  const size_t wordCount = size / 4;
  Array<BindingDecoration> bindingDecorations;
  Array<uint32_t> uniformVariableIds;
  size_t index = 5;
  while (index < wordCount) {
    const uint32_t instruction = readSpirvWord(data, index);
    const uint16_t instructionWordCount =
        static_cast<uint16_t>(instruction >> 16u);
    const uint16_t opCode = static_cast<uint16_t>(instruction & 0xffffu);
    if (instructionWordCount == 0 || index + instructionWordCount > wordCount) {
      break;
    }

    if (opCode == opDecorate && instructionWordCount >= 4) {
      const uint32_t targetId = readSpirvWord(data, index + 1);
      const uint32_t decoration = readSpirvWord(data, index + 2);
      if (decoration == decorationBinding) {
        const uint32_t binding = readSpirvWord(data, index + 3);
        bindingDecorations.push_back({targetId, binding});
      }
    } else if (opCode == opVariable && instructionWordCount >= 4) {
      const uint32_t resultId = readSpirvWord(data, index + 2);
      const uint32_t storageClass = readSpirvWord(data, index + 3);
      if (storageClass == storageClassUniform) {
        pushUniqueId(uniformVariableIds, resultId);
      }
    }

    index += instructionWordCount;
  }

  uint32_t slots = 0;
  for (uint32_t id : uniformVariableIds) {
    const std::optional<uint32_t> binding =
        findBindingDecoration(bindingDecorations, id);
    if (binding) {
      slots = std::max(slots, *binding + 1);
    }
  }
  return slots;
}

std::optional<uint32_t> parseWgslBinding(StringView source,
                                         size_t bindingPos) {
  constexpr StringView bindingPrefix = "@binding(";
  size_t valuePos = bindingPos + bindingPrefix.size();
  if (valuePos >= source.size()) {
    return std::nullopt;
  }

  uint32_t value = 0;
  bool parsedDigit = false;
  while (valuePos < source.size() && source[valuePos] >= '0' &&
         source[valuePos] <= '9') {
    parsedDigit = true;
    value = value * 10 + static_cast<uint32_t>(source[valuePos] - '0');
    valuePos++;
  }
  if (!parsedDigit || valuePos >= source.size() || source[valuePos] != ')') {
    return std::nullopt;
  }
  return value;
}

std::optional<uint32_t> countWgslUniformBufferSlots(const void *data,
                                                    size_t size) {
  if (!data) {
    return std::nullopt;
  }

  StringView source(static_cast<const char *>(data), size);
  constexpr StringView uniformDecl = "var<uniform>";
  constexpr StringView bindingPrefix = "@binding(";
  uint32_t slots = 0;
  uint32_t fallbackCount = 0;
  bool foundBinding = false;
  size_t pos = 0;
  while ((pos = source.find(uniformDecl, pos)) != StringView::npos) {
    fallbackCount++;
    const size_t lineStart = source.rfind('\n', pos);
    const size_t bindingPos = source.rfind(bindingPrefix, pos);
    if (bindingPos != StringView::npos &&
        (lineStart == StringView::npos || bindingPos > lineStart)) {
      const std::optional<uint32_t> binding =
          parseWgslBinding(source, bindingPos);
      if (binding) {
        foundBinding = true;
        slots = std::max(slots, *binding + 1);
      }
    }
    pos += uniformDecl.size();
  }
  if (!foundBinding && fallbackCount > 0) {
    return fallbackCount;
  }
  return slots;
}

void overrideUniformBufferSlotCountFromCode(
    ShaderCompiler::ReflectionData &reflectionData, ShaderFormat format,
    const void *data, size_t size) {
  std::optional<uint32_t> count;
  if (format == ShaderFormat::WGSL) {
    count = countWgslUniformBufferSlots(data, size);
  } else if (format == ShaderFormat::SPIRV ||
             format == ShaderFormat::SPIRV_1_3) {
    count = countSpirvUniformBufferSlots(data, size);
  }
  if (count) {
    reflectionData.numUniformBuffers = *count;
  }
}

bool isParameterLocationUsed(slang::IMetadata *metadata,
                             SlangParameterCategory category,
                             SlangUInt spaceIndex, SlangUInt registerIndex,
                             bool &querySucceeded) {
  bool used = false;
  const SlangResult result = metadata->isParameterLocationUsed(
      category, spaceIndex, registerIndex, used);
  if (SLANG_FAILED(result)) {
    return false;
  }

  querySucceeded = true;
  return used;
}

bool isUnknownBindingValue(unsigned value) {
  return value == static_cast<unsigned>(SLANG_UNKNOWN_SIZE) ||
         value == static_cast<unsigned>(SLANG_UNBOUNDED_SIZE);
}

bool hasCategory(slang::VariableLayoutReflection *varLayout,
                 slang::ParameterCategory category) {
  const unsigned int categoryCount = varLayout->getCategoryCount();
  for (unsigned int i = 0; i < categoryCount; ++i) {
    if (varLayout->getCategoryByIndex(i) == category) {
      return true;
    }
  }
  return varLayout->getCategory() == category;
}

bool isUniformBufferParameter(slang::VariableLayoutReflection *varLayout) {
  return hasCategory(varLayout, slang::ParameterCategory::ConstantBuffer);
}

bool isVariableLocationUsed(slang::IMetadata *metadata,
                            slang::VariableLayoutReflection *varLayout) {
  if (!metadata) {
    return true;
  }

  const unsigned bindingIndex = varLayout->getBindingIndex();
  const unsigned bindingSpace = varLayout->getBindingSpace();
  if (isUnknownBindingValue(bindingIndex) || isUnknownBindingValue(bindingSpace)) {
    return true;
  }

  bool querySucceeded = false;
  if (isParameterLocationUsed(metadata, SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER,
                              bindingSpace, bindingIndex, querySucceeded)) {
    return true;
  }
  if (isParameterLocationUsed(metadata, SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT,
                              bindingSpace, bindingIndex, querySucceeded)) {
    return true;
  }

  return !querySucceeded;
}

bool isBindingLocationUsed(slang::IMetadata *metadata,
                           slang::TypeLayoutReflection *typeLayout,
                           SlangInt bindingRangeIndex) {
  if (!metadata) {
    return true;
  }

  const SlangInt descriptorRangeCount =
      typeLayout->getBindingRangeDescriptorRangeCount(bindingRangeIndex);
  if (!isKnownNonNegative(descriptorRangeCount)) {
    return true;
  }
  if (descriptorRangeCount == 0) {
    return true;
  }

  const SlangInt descriptorSetIndex =
      typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
  const SlangInt firstDescriptorRangeIndex =
      typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
  if (!isKnownNonNegative(descriptorSetIndex) ||
      !isKnownNonNegative(firstDescriptorRangeIndex)) {
    return true;
  }

  const SlangInt spaceIndex =
      typeLayout->getDescriptorSetSpaceOffset(descriptorSetIndex);
  if (!isKnownNonNegative(spaceIndex)) {
    return true;
  }

  bool checkedAnyDescriptor = false;
  for (SlangInt i = 0; i < descriptorRangeCount; ++i) {
    const SlangInt descriptorRangeIndex = firstDescriptorRangeIndex + i;
    const auto category = typeLayout->getDescriptorSetDescriptorRangeCategory(
        descriptorSetIndex, descriptorRangeIndex);
    const SlangInt registerIndex =
        typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
            descriptorSetIndex, descriptorRangeIndex);
    const SlangInt descriptorCount =
        typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
            descriptorSetIndex, descriptorRangeIndex);
    if (!isKnownNonNegative(registerIndex) ||
        !isKnownNonNegative(descriptorCount)) {
      return true;
    }

    for (SlangInt descriptorIndex = 0; descriptorIndex < descriptorCount;
         ++descriptorIndex) {
      bool used = false;
      bool querySucceeded = false;
      used = isParameterLocationUsed(
          metadata, static_cast<SlangParameterCategory>(category),
          static_cast<SlangUInt>(spaceIndex),
          static_cast<SlangUInt>(registerIndex + descriptorIndex),
          querySucceeded);
      if (!used && category != slang::ParameterCategory::DescriptorTableSlot) {
        used = isParameterLocationUsed(
            metadata, SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT,
            static_cast<SlangUInt>(spaceIndex),
            static_cast<SlangUInt>(registerIndex + descriptorIndex),
            querySucceeded);
      }
      if (!querySucceeded) {
        return true;
      }
      checkedAnyDescriptor = true;
      if (used) {
        return true;
      }
    }
  }

  return !checkedAnyDescriptor;
}

void countBindingRange(ShaderCompiler::ReflectionData &data,
                       slang::BindingType rangeType) {
  if (rangeType == slang::BindingType::CombinedTextureSampler ||
      rangeType == slang::BindingType::Sampler) {
    data.numCombinedSamplers++;
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
} // namespace

ShaderCompiler::ReflectionData
getReflectionData(slang::IComponentType *program, slang::IMetadata *metadata) {
  auto *programLayout = program->getLayout();
  ShaderCompiler::ReflectionData data;

  if (!programLayout)
    return data;

  {

    slang::TypeLayoutReflection *typeLayout =
        programLayout->getGlobalParamsTypeLayout();
    const int rangeCount = typeLayout->getBindingRangeCount();

    for (int i = 0; i < rangeCount; ++i) {
      if (!isBindingLocationUsed(metadata, typeLayout, i)) {
        continue;
      }
      auto rangeType = typeLayout->getBindingRangeType(i);
      countBindingRange(data, rangeType);
    }
  }

  const unsigned int parameterCount = programLayout->getParameterCount();
  for (unsigned int i = 0; i < parameterCount; ++i) {
    auto *varLayout = programLayout->getParameterByIndex(i);
    if (!varLayout || !isUniformBufferParameter(varLayout)) {
      continue;
    }
    if (!isVariableLocationUsed(metadata, varLayout)) {
      continue;
    }
    data.numUniformBuffers++;
  }

  return data;
}

Array<char> ShaderCompiler::compileSource(StringView moduleName,
                                          StringView modulePath,
                                          StringView source, ShaderStage stage,
                                          ShaderFormat format,
                                          ReflectionData &reflectionData) {
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
  case ShaderFormat::SPIRV_1_3:
    targetDesc[0].format = SLANG_SPIRV;
    targetDesc[0].profile = globalSession->findProfile("spirv_1_3");
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

  Slang::ComPtr<slang::IMetadata> metadata;
  {
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    SlangResult result =
        linkedProgram->getTargetMetadata(0, metadata.writeRef(),
                                         diagnosticsBlob.writeRef());
    if (SLANG_FAILED(result)) {
      diagnoseIfNeeded(diagnosticsBlob);
      diagnosticsBlob.setNull();
      linkedProgram->getEntryPointMetadata(0, 0, metadata.writeRef(),
                                           diagnosticsBlob.writeRef());
      diagnoseIfNeeded(diagnosticsBlob);
    }
  }

  reflectionData = getReflectionData(linkedProgram, metadata);
  overrideUniformBufferSlotCountFromCode(reflectionData, format,
                                         compiledCode->getBufferPointer(),
                                         compiledCode->getBufferSize());
  Array<char> shaderData(compiledCode->getBufferSize());
  std::memcpy(shaderData.data(), compiledCode->getBufferPointer(),
              compiledCode->getBufferSize());
  return shaderData;
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
  auto source = sinen::AssetReader::readAsString(sourcePath);
  auto modulePath = sinen::AssetReader::getLoadPath(sourcePath);
  return compileSource(moduleName, modulePath, source, stage, format,
                       reflectionData);
}
} // namespace sinen
