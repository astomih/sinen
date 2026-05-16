#include "webgpu_shader.hpp"

#include <string_view>

namespace sinen::gpu::webgpu {
namespace {
bool parseUint(const char *begin, const char *end, UInt32 &value) {
  if (begin >= end || *begin < '0' || *begin > '9') {
    return false;
  }
  UInt32 result = 0;
  for (auto *p = begin; p < end; ++p) {
    if (*p < '0' || *p > '9') {
      return false;
    }
    result = result * 10u + static_cast<UInt32>(*p - '0');
  }
  value = result;
  return true;
}

bool parseAttributeValue(std::string_view source, size_t attrPos,
                         const char *name, UInt32 &value) {
  const std::string_view attrName(name);
  if (attrPos + attrName.size() >= source.size() ||
      source.substr(attrPos, attrName.size()) != attrName) {
    return false;
  }
  const size_t open = attrPos + attrName.size();
  const size_t close = source.find(')', open);
  if (close == std::string_view::npos) {
    return false;
  }
  return parseUint(source.data() + open, source.data() + close, value);
}
} // namespace

void Shader::initializeUniformBindings(const CreateInfo &createInfo) {
  if (createInfo.format != ShaderFormat::WGSL || !createInfo.data ||
      createInfo.size == 0) {
    return;
  }

  knownUniformBindings = true;
  std::string_view source(static_cast<const char *>(createInfo.data),
                          createInfo.size);
  size_t varPos = 0;
  while ((varPos = source.find("var", varPos)) != std::string_view::npos) {
    const size_t lineStart = source.rfind('\n', varPos);
    const size_t searchStart =
        lineStart == std::string_view::npos ? 0 : lineStart + 1;
    const size_t lineEnd = source.find('\n', varPos);
    const size_t searchEnd =
        lineEnd == std::string_view::npos ? source.size() : lineEnd;
    const std::string_view line =
        source.substr(searchStart, searchEnd - searchStart);
    const bool isUniform = line.find("var<uniform>") != std::string_view::npos;
    const bool isResource = line.find("texture_") != std::string_view::npos ||
                            line.find(": sampler") != std::string_view::npos;
    if (!isUniform && !isResource) {
      ++varPos;
      continue;
    }

    UInt32 group = 0;
    UInt32 binding = 0;
    bool hasGroup = false;
    bool hasBinding = false;
    size_t attrPos = 0;
    while ((attrPos = line.find('@', attrPos)) != std::string_view::npos) {
      ++attrPos;
      if (parseAttributeValue(line, attrPos, "group(", group)) {
        hasGroup = true;
      } else if (parseAttributeValue(line, attrPos, "binding(", binding)) {
        hasBinding = true;
      }
    }

    if (hasGroup && hasBinding) {
      if (isUniform) {
        uniformBindings.emplace_back(group, binding);
      } else {
        resourceBindings.emplace_back(group, binding);
      }
    }
    ++varPos;
  }
}

Shader::~Shader() {
  if (shaderModule) {
    wgpuShaderModuleRelease(shaderModule);
    shaderModule = nullptr;
  }
}

bool Shader::hasUniformBinding(UInt32 group, UInt32 binding) const {
  for (const auto &[uniformGroup, uniformBinding] : uniformBindings) {
    if (uniformGroup == group && uniformBinding == binding) {
      return true;
    }
  }
  return false;
}

bool Shader::hasResourceBinding(UInt32 group, UInt32 binding) const {
  for (const auto &[resourceGroup, resourceBinding] : resourceBindings) {
    if (resourceGroup == group && resourceBinding == binding) {
      return true;
    }
  }
  return false;
}
} // namespace sinen::gpu::webgpu
