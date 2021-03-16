#pragma once
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

class PipelineVK
{
public:
	PipelineVK() = default;
	void prepare();

private:
	VkPipeline m_pipeline;
};