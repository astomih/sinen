#include <PipelineLayout.h>
#include <Vertex.h>

namespace nen::vk
{
    PipelineLayout::PipelineLayout()
    {
        /*
        // Setting vertex inputs
        VkVertexInputBindingDescription inputBinding{
            0,                          // binding
            sizeof(Vertex),             // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate
        };
        std::array<VkVertexInputBindingDescription, 2> vibDisc{
            {
                {0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX},
                {1, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_INSTANCE},
            }};
        std::array<VkVertexInputAttributeDescription, 3> inputAttribs{
            {
                {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
                {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm)},
                {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, uv)},
            }};
        VkPipelineVertexInputStateCreateInfo vertexInputCI{};
        vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCI.vertexBindingDescriptionCount = 1;
        vertexInputCI.pVertexBindingDescriptions = &inputBinding;
        vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
        vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();

        // Setting blending
        const auto colorWriteAll =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.colorWriteMask = colorWriteAll;
        VkPipelineColorBlendStateCreateInfo cbCI{};
        cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cbCI.attachmentCount = 1;
        cbCI.pAttachments = &blendAttachment;

        // Set viewport
        VkViewport viewport;
        {
            viewport.x = 0.0f;
            viewport.y = float(m_base->mSwapchain->GetSurfaceExtent().height);
            viewport.width = float(m_base->mSwapchain->GetSurfaceExtent().width);
            viewport.height = -1.0f * float(m_base->mSwapchain->GetSurfaceExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }
        VkRect2D scissor = {
            {0, 0}, // offset
            m_base->mSwapchain->GetSurfaceExtent()};
        VkPipelineViewportStateCreateInfo viewportCI{};
        viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCI.viewportCount = 1;
        viewportCI.pViewports = &viewport;
        viewportCI.scissorCount = 1;
        viewportCI.pScissors = &scissor;

        // Setting primitive toporogy
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
        inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Setting rasterizer state
        VkPipelineRasterizationStateCreateInfo rasterizerCI{};
        rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerCI.cullMode = VK_CULL_MODE_NONE;
        rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerCI.lineWidth = 1.0f;

        // Setting multisampling
        VkPipelineMultisampleStateCreateInfo multisampleCI{};
        multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Setting depth stencil state
        VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
        depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCI.depthTestEnable = VK_TRUE;
        depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilCI.depthWriteEnable = VK_TRUE;
        depthStencilCI.stencilTestEnable = VK_FALSE;

        // Setting pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
        vkCreatePipelineLayout(m_base->m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout);
        */
    }
}