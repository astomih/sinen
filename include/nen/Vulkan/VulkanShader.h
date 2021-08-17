#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <vulkan/vk_layer.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif
#include <vector>
namespace nen::vk
{
    class VulkanShader
    {
    public:
        VulkanShader() = default;
        static VkPipelineShaderStageCreateInfo LoadModule(VkDevice device, const char *fileName, VkShaderStageFlagBits stage);
        static void CleanModule(VkDevice device, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);

    private:
    };
}
#endif