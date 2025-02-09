#include "vk_shader.hpp"
#include <fstream>
#include <io/data_stream.hpp>

/*
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
*/

namespace sinen {
VkPipelineShaderStageCreateInfo vk_shader::load(VkDevice device,
                                                const char *fileName,
                                                VkShaderStageFlagBits stage) {
  if (std::string(fileName).ends_with(".spv")) {

    std::string filedata =
        DataStream::open_as_string(AssetType::Shader, fileName);

    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.pCode = reinterpret_cast<uint32_t *>(filedata.data());
    ci.codeSize = filedata.size();
    vkCreateShaderModule(device, &ci, nullptr, &shaderModule);
    VkPipelineShaderStageCreateInfo shaderStageCI{};
    shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCI.stage = stage;
    shaderStageCI.module = shaderModule;
    shaderStageCI.pName = "main";
    return shaderStageCI;
  } else {
    /*
    // Compile glsl to SPIR-V
    std::string filedata =
        DataStream::open_as_string(AssetType::Shader, fileName);

    glslang::InitializeProcess();
    EShLanguage lang;
    if (std::string(fileName).ends_with(".vert")) {
      lang = EShLangVertex;
    } else if (std::string(fileName).ends_with(".frag")) {
      lang = EShLangFragment;
    } else if (std::string(fileName).ends_with(".comp")) {
      lang = EShLangCompute;
    } else {
      throw std::runtime_error("Unknown shader type");
    }

    glslang::TShader shader(lang);
    const char *shaderStrings[1];
    shaderStrings[0] = filedata.c_str();
    shader.setStrings(shaderStrings, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan,
                       100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    const TBuiltInResource *resources = GetDefaultResources();
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    std::string preprocessedGLSL;
    if (!shader.parse(resources, 100, false, messages)) {
      throw std::runtime_error("GLSL parsing failed");
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
      throw std::runtime_error("GLSL linking failed");
    }
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(lang), spirv);
    glslang::FinalizeProcess();

    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.pCode = spirv.data();
    ci.codeSize = spirv.size() * sizeof(uint32_t);
    vkCreateShaderModule(device, &ci, nullptr, &shaderModule);
    VkPipelineShaderStageCreateInfo shaderStageCI{};
    shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCI.stage = stage;
    shaderStageCI.module = shaderModule;
    shaderStageCI.pName = "main";
    return shaderStageCI;
  */
  }
}
void vk_shader::clean(
    VkDevice device,
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) {
  for (const auto &v : shaderStages) {
    vkDestroyShaderModule(device, v.module, nullptr);
  }
}
} // namespace sinen