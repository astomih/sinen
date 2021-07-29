#include <Engine.hpp>

namespace nen::vk
{
    void Pipeline::Initialize(PipelineLayout &layout, VkRenderPass renderPass, std::vector<VkPipelineShaderStageCreateInfo>& shaderStages)
    {
        // ブレンディングの設定
        const auto colorWriteAll =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        blendAttachment = VkPipelineColorBlendAttachmentState{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = colorWriteAll};
        cbCI = VkPipelineColorBlendStateCreateInfo{};
        cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cbCI.attachmentCount = 1;
        cbCI.pAttachments = &blendAttachment;

        // デプスステンシルステート設定
        depthStencilCI = VkPipelineDepthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .stencilTestEnable = VK_FALSE};

        // パイプラインの構築
        graphicsCI = VkGraphicsPipelineCreateInfo{};
        graphicsCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsCI.stageCount = uint32_t(shaderStages.size());
        graphicsCI.pStages = shaderStages.data();
        graphicsCI.pInputAssemblyState = layout.GetInputAssemblyCI();
        graphicsCI.pVertexInputState = layout.GetVertexInputCI();
        graphicsCI.pRasterizationState = layout.GetRasterizerCI();
        graphicsCI.pDepthStencilState = layout.GetDepthStencilCI();
        graphicsCI.pMultisampleState = layout.GetMultisampleCI();
        graphicsCI.pViewportState = layout.GetViewportCI();
        graphicsCI.pColorBlendState = &cbCI;
        graphicsCI.renderPass = renderPass;
        graphicsCI.layout = layout.GetLayout();
    }
    void Pipeline::Prepare(VkDevice device)
    {
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsCI, nullptr, &pipeline);
    }

    void Pipeline::Bind(VkCommandBuffer command)
    {
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void Pipeline::Cleanup(VkDevice device)
    {
        nen::vkutil::DestroyVulkanObject<VkPipeline>(device, pipeline, &vkDestroyPipeline);
    }

    void Pipeline::SetDepthTest(VkBool32 isEnable)
    {
        depthStencilCI.depthTestEnable = isEnable;
    }
    void Pipeline::SetDepthWrite(VkBool32 isEnable)
    {
        depthStencilCI.depthWriteEnable = isEnable;
    }

    void Pipeline::ColorBlendFactor(VkBlendFactor src, VkBlendFactor dst)
    {
        blendAttachment.srcColorBlendFactor = src;
        blendAttachment.dstColorBlendFactor = dst;
    }
    void Pipeline::AlphaBlendFactor(VkBlendFactor src, VkBlendFactor dst)
    {
        blendAttachment.srcAlphaBlendFactor = src;
        blendAttachment.dstAlphaBlendFactor = dst;
    }
}