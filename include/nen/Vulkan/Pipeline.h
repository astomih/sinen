#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <vulkan/vk_layer.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif
#include <vector>
#include "PipelineLayout.h"
namespace nen::vk
{
	class Pipeline
	{
	public:
		Pipeline() = default;
		void Initialize(
			PipelineLayout &layout,
			VkRenderPass renderPass,
			std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);
		void Prepare(VkDevice device);
		void Bind(VkCommandBuffer command);
		void Cleanup(VkDevice device);

		void SetDepthTest(VkBool32 isEnable);
		void SetDepthWrite(VkBool32 isEnable);
		void ColorBlendFactor(VkBlendFactor src, VkBlendFactor dst);
		void AlphaBlendFactor(VkBlendFactor src, VkBlendFactor dst);

	private:
		VkPipeline pipeline;
		VkPipelineColorBlendStateCreateInfo cbCI;
		VkPipelineColorBlendAttachmentState blendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilCI;
		VkGraphicsPipelineCreateInfo graphicsCI;
	};
}
#endif