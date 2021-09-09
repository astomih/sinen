#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#ifdef _WIN32
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vk_layer.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif // _WIN32
#include <vector>
#include <memory>
#include "Swapchain.h"
#include <Window/Window.hpp>
namespace nen::vk
{
	class VKBase
	{
	public:
		VKBase(class VKRenderer *);
		void initialize(std::shared_ptr<Window> window);
		void terminate();

		void render();
		VkPhysicalDevice GetVkPhysicalDevice() { return m_physDev; }
		VkDevice GetVkDevice() { return m_device; }
		VkQueue GetVkQueue() { return m_deviceQueue; }
		VkCommandPool GetVkCommandPool() { return m_commandPool; }
		int32_t GetSwapBufferCount() { return static_cast<int32_t>(mSwapchain->GetImageCount()); }

		static void checkResult(VkResult);

		void initializeInstance(const char *appName);
		void selectPhysicalDevice();
		uint32_t searchGraphicsQueueIndex();
		void createDevice();
		void prepareCommandPool();
		void createDepthBuffer();

		void createRenderPass();
		void createFramebuffer();

		void prepareCommandBuffers();

		uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps) const;

		void enableDebugReport();
		void disableDebugReport();

		std::unique_ptr<Swapchain> mSwapchain;

		VkInstance m_instance;
		VkDevice m_device;
		VkPhysicalDevice m_physDev;

		VkPhysicalDeviceMemoryProperties m_physMemProps;

		uint32_t m_graphicsQueueIndex;
		VkQueue m_deviceQueue;

		VkCommandPool m_commandPool;

		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;

		std::vector<VkFence> m_fences;
		VkSemaphore m_renderCompletedSem, m_presentCompletedSem;

		PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallbackEXT;
		PFN_vkDebugReportMessageEXT m_vkDebugReportMessageEXT;
		PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallbackEXT;
		VkDebugReportCallbackEXT m_debugReport;

		std::vector<VkCommandBuffer> m_commands;

		std::vector<VkDeviceMemory> destroyMemory;

		uint32_t m_imageIndex;
		std::shared_ptr<Window> m_window;
		VKRenderer *m_vkrenderer;
	};
}

#endif // EMSCRIPTEN