#include <Engine.hpp>

namespace nen::vk
{
    VkPipelineShaderStageCreateInfo VulkanShader::LoadModule(VkDevice device, const char *fileName, VkShaderStageFlagBits stage)
    {
        std::ifstream infile(fileName, std::ios::binary);
        if (!infile)
        {
            OutputDebugStringA("file not found.\n");
            DebugBreak();
        }
        std::vector<char> filedata;
        filedata.resize(uint32_t(infile.seekg(0, std::ifstream::end).tellg()));
        infile.seekg(0, std::ifstream::beg).read(filedata.data(), filedata.size());

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
    }

    void VulkanShader::CleanModule(VkDevice device, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages)
    {
        for (const auto &v : shaderStages)
        {
            vkDestroyShaderModule(device, v.module, nullptr);
        }
    }
}