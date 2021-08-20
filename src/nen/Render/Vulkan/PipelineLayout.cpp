#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "VKBase.h"
#include "Pipeline.h"
#include "VulkanUtil.h"
#include <nen.hpp>

namespace nen::vk
{
    void PipelineLayout::Initialize(VkDevice device, const VkDescriptorSetLayout *descriptorLayout, const VkExtent2D &extent)
    {
        // Setting vertex inputs
        inputBinding = {
            0,                          // binding
            sizeof(Vertex),             // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate
        };
        vibDisc = {
            {
                {0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX},
                {1, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_INSTANCE},
            }};
        inputAttribs = {
            {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm)},
                {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, uv)},
            }};

        vertexInputCI = VkPipelineVertexInputStateCreateInfo{};
        vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCI.vertexBindingDescriptionCount = 1;
        vertexInputCI.pVertexBindingDescriptions = &inputBinding;
        vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
        vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();

        // Setting blending
        colorMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        blendAttachment = VkPipelineColorBlendAttachmentState{};
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.colorWriteMask = colorMask;

        cbCI = VkPipelineColorBlendStateCreateInfo{};
        cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cbCI.attachmentCount = 1;
        cbCI.pAttachments = &blendAttachment;

        // Set viewport
        {
            viewport = VkViewport{};
            viewport.x = 0.0f;
            viewport.y = float(extent.height);
            viewport.width = float(extent.width);
            viewport.height = -1.0f * float(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }

        scissor = {
            {0, 0}, // offset
            extent};
        viewportCI = VkPipelineViewportStateCreateInfo{};
        viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCI.viewportCount = 1;
        viewportCI.pViewports = &viewport;
        viewportCI.scissorCount = 1;
        viewportCI.pScissors = &scissor;

        // Setting primitive toporogy
        inputAssemblyCI = VkPipelineInputAssemblyStateCreateInfo{};
        inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Setting rasterizer state
        rasterizerCI = VkPipelineRasterizationStateCreateInfo{};
        rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerCI.cullMode = VK_CULL_MODE_NONE;
        rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerCI.lineWidth = 1.0f;

        // Setting multisampling
        multisampleCI = VkPipelineMultisampleStateCreateInfo{};
        multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Setting depth stencil state
        depthStencilCI = VkPipelineDepthStencilStateCreateInfo{};
        depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCI.depthTestEnable = VK_TRUE;
        depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilCI.depthWriteEnable = VK_TRUE;
        depthStencilCI.stencilTestEnable = VK_FALSE;

        // Setting pipeline layout
        pipelineLayoutCI = VkPipelineLayoutCreateInfo{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        if (descriptorLayout)
            pipelineLayoutCI.pSetLayouts = descriptorLayout;
    }

    void PipelineLayout::Prepare(VkDevice device)
    {
        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCI, NULL, &layout);
    }

    void PipelineLayout::Cleanup(VkDevice device)
    {
        nen::vkutil::DestroyVulkanObject<VkPipelineLayout>(device, layout, &vkDestroyPipelineLayout);
    }
}
#endif