﻿#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "EffectManagerVK.h"
#include "VKBase.h"
#include "VKRenderer.h"
#include "VulkanUtil.h"
#include "VulkanShader.h"
#include <Nen.hpp>
#include <fstream>
#include <array>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <sstream>
#include <Effekseer.h>
#include <EffekseerRendererVulkan.h>

namespace nen::vk
{
	using namespace vkutil;

	constexpr int maxpoolSize = 2000;
	constexpr int maxInstanceCount = 900;
	VKRenderer::VKRenderer()
		: m_descriptorPool(),
		  m_descriptorSetLayout(),
		  m_sampler(),
		  m_base(std::make_unique<VKBase>(this)),
		  instance(maxInstanceCount)
	{
	}
	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	void VKRenderer::Initialize(SDL_Window *window)
	{
		m_base->initialize(window, Window::name.c_str());
		mEffectManager = std::make_unique<EffectManagerVK>(this);
		mEffectManager->Init();
	}

	void VKRenderer::Shutdown()
	{
		cleanup();
		m_base->terminate();
	}
	void VKRenderer::Render()
	{
		m_base->render();
	}
	void VKRenderer::AddVertexArray(const VertexArray &vArray, std::string_view name)
	{
		VertexArrayForVK vArrayVK;
		vArrayVK.indexCount = vArray.indexCount;
		vArrayVK.indices = vArray.indices;
		vArrayVK.vertices = vArray.vertices;
		auto vArraySize = vArray.vertices.size() * sizeof(Vertex);
		vArrayVK.vertexBuffer = CreateBuffer(vArraySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vArrayVK.indexBuffer = CreateBuffer(vArray.indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		MapMemory(vArrayVK.vertexBuffer.memory, vArrayVK.vertices.data(), vArraySize);
		MapMemory(vArrayVK.indexBuffer.memory, vArrayVK.indices.data(), sizeof(uint32_t) * vArrayVK.indices.size());
		m_VertexArrays.insert(std::pair<std::string, VertexArrayForVK>(name.data(), vArrayVK));
	}
	void VKRenderer::ChangeBufferSprite(std::shared_ptr<class Sprite> sprite, const TextureType type)
	{
		if (type == TextureType::Image2D)
		{
			auto &sprites = GetSprite2Ds();
			for (auto &i : sprites)
			{
				if (sprite == i->sprite)
				{
					if (sprite->isChangeBuffer && i->buffer.buffer == 0)
					{
						i->buffer = CreateBuffer(
							sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
						break;
					}
				}
			}
		}
		if (type == TextureType::Image3D)
		{
			auto &sprites = GetSprite3Ds();
			for (auto &i : sprites)
			{
				if (sprite == i->sprite)
				{
					if (sprite->isChangeBuffer && i->buffer.buffer == 0)
					{
						i->buffer = CreateBuffer(
							sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
						break;
					}
				}
			}
		}
	}

	void VKRenderer::AddSprite2D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture)
	{
		auto t = std::make_shared<vk::SpriteVK>();
		if (sprite->isChangeBuffer)
			t->buffer = CreateBuffer(
				sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		t->sprite = sprite;
		registerImageObject(texture);
		t->mTexture = texture;
		registerTexture(t, texture->id, TextureType::Image2D);
	}
	void VKRenderer::RemoveSprite2D(std::shared_ptr<class Sprite> sprite)
	{
		auto &sprites = GetSprite2Ds();
		auto iter = sprites.begin();
		for (;
			 iter != sprites.end();)
		{
			if (sprite == (*iter)->sprite)
			{
				unregisterTexture((*iter), TextureType::Image2D);
				iter = sprites.begin();
				if (iter == sprites.end())
					break;
			}
			iter++;
		}
	}

	void VKRenderer::AddSprite3D(std::shared_ptr<class Sprite> sprite, std::shared_ptr<Texture> texture)
	{
		auto t = std::make_shared<vk::SpriteVK>();
		t->sprite = sprite;
		registerImageObject(texture);
		t->mTexture = texture;
		registerTexture(t, texture->id, TextureType::Image3D);
	}
	void VKRenderer::RemoveSprite3D(std::shared_ptr<class Sprite> sprite)
	{
		auto &sprites = GetSprite3Ds();
		auto iter = sprites.begin();
		for (; iter != sprites.end(); ++iter)
		{
			if (sprite == (*iter)->sprite)
			{
				unregisterTexture((*iter), TextureType::Image3D);
				iter = sprites.begin();
				if (iter == sprites.end())
					break;
			}
		}
	}

	void VKRenderer::prepare()
	{
		createBoxVertices();
		createSpriteVertices();
		prepareUniformBuffers();
		prepareDescriptorSetLayout();
		prepareDescriptorPool();

		m_sampler = createSampler();
		prepareDescriptorSetAll();

		mPipelineLayout.Initialize(m_base->GetVkDevice(), &m_descriptorSetLayout, m_base->mSwapchain->GetSurfaceExtent());
		mPipelineLayout.Prepare(m_base->GetVkDevice());

		// 不透明用: パイプラインの構築
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shaderOpaque.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
			pipelineOpaque.Initialize(mPipelineLayout, m_base->m_renderPass, shaderStages);
			pipelineOpaque.ColorBlendFactor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
			pipelineOpaque.AlphaBlendFactor(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
			pipelineOpaque.Prepare(m_base->GetVkDevice());
			VulkanShader::CleanModule(m_base->GetVkDevice(), shaderStages);
		}

		// 半透明用: パイプラインの構築
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shaderAlpha.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
			pipelineAlpha.Initialize(mPipelineLayout, m_base->m_renderPass, shaderStages);
			pipelineAlpha.ColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			pipelineAlpha.AlphaBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			pipelineAlpha.SetDepthTest(VK_TRUE);
			pipelineAlpha.SetDepthWrite(VK_FALSE);
			pipelineAlpha.Prepare(m_base->GetVkDevice());
			VulkanShader::CleanModule(m_base->GetVkDevice(), shaderStages);
		}
		// 2D用: パイプラインの構築
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
				VulkanShader::LoadModule(m_base->GetVkDevice(), "Assets/Shader/Vulkan/shaderAlpha.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
			pipeline2D.Initialize(mPipelineLayout, m_base->m_renderPass, shaderStages);
			pipeline2D.ColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			pipeline2D.AlphaBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ZERO);
			pipeline2D.SetDepthTest(VK_FALSE);
			pipeline2D.SetDepthWrite(VK_FALSE);
			pipeline2D.Prepare(m_base->GetVkDevice());
			VulkanShader::CleanModule(m_base->GetVkDevice(), shaderStages);
		}
		prepareImGUI();
	}
	template <class T>
	inline bool isExist(T &handle)
	{
		return handle != VK_NULL_HANDLE;
	}
	void VKRenderer::cleanup()
	{
		VkDevice device = m_base->GetVkDevice();
		for (auto &i : mImageObjects)
		{
			DestroyVulkanObject<VkImage>(device, i.second.image, &vkDestroyImage);
			DestroyVulkanObject<VkImageView>(device, i.second.view, &vkDestroyImageView);
			DestroyVulkanObject<VkDeviceMemory>(device, i.second.view, &vkFreeMemory);
		}
		DestroyVulkanObject<VkSampler>(device, m_sampler, &vkDestroySampler);
		mPipelineLayout.Cleanup(device);
		pipelineOpaque.Cleanup(device);
		pipelineAlpha.Cleanup(device);
		pipeline2D.Cleanup(device);
		for (auto &i : m_VertexArrays)
		{
			DestroyVulkanObject<VkBuffer>(device, i.second.vertexBuffer.buffer, &vkDestroyBuffer);
			DestroyVulkanObject<VkBuffer>(device, i.second.indexBuffer.buffer, &vkDestroyBuffer);
			DestroyVulkanObject<VkDeviceMemory>(device, i.second.vertexBuffer.memory, &vkFreeMemory);
			DestroyVulkanObject<VkDeviceMemory>(device, i.second.indexBuffer.memory, &vkFreeMemory);
		}
		DestroyVulkanObject<VkDescriptorPool>(device, m_descriptorPool, &vkDestroyDescriptorPool);
		DestroyVulkanObject<VkDescriptorSetLayout>(device, m_descriptorSetLayout, &vkDestroyDescriptorSetLayout);
	}
	void VKRenderer::makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri, VkCommandBufferBeginInfo &ci, VkFence &fence)
	{
		vkWaitForFences(m_base->GetVkDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
		vkBeginCommandBuffer(command, &ci);
		vkCmdBeginRenderPass(command, &ri, VK_SUBPASS_CONTENTS_INLINE);
		draw3d(command);
		draw2d(command);
		renderEffekseer(command);
		renderImGUI(command);
		pipelineOpaque.Bind(command);
	}

	void VKRenderer::draw3d(VkCommandBuffer command)
	{
		pipelineOpaque.Bind(command);
		VkDeviceSize offset = 0;
		for (auto &sprite : mTextures3D)
		{
			::vkCmdBindVertexBuffers(command, 0, 1, &m_VertexArrays[sprite->sprite->vertexIndex].vertexBuffer.buffer, &offset);
			::vkCmdBindIndexBuffer(command, m_VertexArrays[sprite->sprite->vertexIndex].indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);
			// Set descriptors
			vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout.GetLayout(), 0, 1, &sprite->descripterSet[m_base->m_imageIndex], 0, nullptr);
			{
				auto memory = sprite->uniformBuffers[m_base->m_imageIndex].memory;
				void *p;
				vkMapMemory(m_base->GetVkDevice(), memory, 0, VK_WHOLE_SIZE, 0, &p);
				memcpy(p, &sprite->sprite->param, sizeof(ShaderParameters));
				vkUnmapMemory(m_base->GetVkDevice(), memory);
			}
			vkCmdDrawIndexed(command, m_VertexArrays[sprite->sprite->vertexIndex].indexCount, 1, 0, 0, 0);
		}
	}

	void VKRenderer::draw2d(VkCommandBuffer command)
	{
		pipeline2D.Bind(command);
		VkDeviceSize offset = 0;
		for (auto &sprite : mTextures2D)
		{
			vkCmdBindVertexBuffers(command, 0, 1, &m_VertexArrays[sprite->sprite->vertexIndex].vertexBuffer.buffer, &offset);
			vkCmdBindIndexBuffer(command, m_VertexArrays[sprite->sprite->vertexIndex].indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

			// Set descriptors
			vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout.GetLayout(), 0, 1, &sprite->descripterSet[m_base->m_imageIndex], 0, nullptr);
			{
				auto memory = sprite->uniformBuffers[m_base->m_imageIndex].memory;
				void *p;
				vkMapMemory(m_base->GetVkDevice(), memory, 0, VK_WHOLE_SIZE, 0, &p);
				memcpy(p, &sprite->sprite->param, sizeof(ShaderParameters));
				vkUnmapMemory(m_base->GetVkDevice(), memory);
			}
			vkCmdDrawIndexed(command, m_VertexArrays[sprite->sprite->vertexIndex].indexCount, 1, 0, 0, 0);
		}
	}

	void VKRenderer::MapMemory(VkDeviceMemory memory, void *data, size_t size)
	{

		void *p;
		vkMapMemory(m_base->GetVkDevice(), memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, data, size);
		vkUnmapMemory(m_base->GetVkDevice(), memory);
	}

	void VKRenderer::createBoxVertices()
	{
		const float value = 1.f;
		const Vector2 lb(0.0f, 0.0f);
		const Vector2 lt(0.f, 1.f);
		const Vector2 rb(1.0f, 0.0f);
		const Vector2 rt(1.0f, 1.0f);
		std::array<float, 3> norm = {1, 1, 1};
		const std::array<float, 3> red{1.0f, 0.0f, 0.0f};
		const std::array<float, 3> green{0.0f, 1.0f, 0.0f};
		const std::array<float, 3> blue{0.0f, 0.0f, 1.0f};
		const std::array<float, 3> white{1.0f, 1, 1};
		const std::array<float, 3> black{0.0f, 0, 0};
		const std::array<float, 3> yellow{1.0f, 1.0f, 0.0f};
		const std::array<float, 3> magenta{1.0f, 0.0f, 1.0f};
		const std::array<float, 3> cyan{0.0f, 1.0f, 1.0f};

		VertexArrayForVK vArray;
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, lb});
		vArray.vertices.push_back({Vector3(-value, -value, value), red, lt});
		vArray.vertices.push_back({Vector3(value, value, value), white, rb});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, rt});

		vArray.vertices.push_back({Vector3(value, value, value), white, lb});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, lt});
		vArray.vertices.push_back({Vector3(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, rt});

		vArray.vertices.push_back({Vector3(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, rb});
		vArray.vertices.push_back({Vector3(-value, -value, value), red, rt});

		vArray.vertices.push_back({Vector3(value, value, -value), cyan, lb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, lt});
		vArray.vertices.push_back({Vector3(-value, value, -value), green, rb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, rt});

		vArray.vertices.push_back({Vector3(-value, value, -value), green, lb});
		vArray.vertices.push_back({Vector3(-value, value, value), yellow, lt});
		vArray.vertices.push_back({Vector3(value, value, -value), cyan, rb});
		vArray.vertices.push_back({Vector3(value, value, value), white, rt});

		vArray.vertices.push_back({Vector3(-value, -value, value), red, lb});
		vArray.vertices.push_back({Vector3(-value, -value, -value), black, lt});
		vArray.vertices.push_back({Vector3(value, -value, value), magenta, rb});
		vArray.vertices.push_back({Vector3(value, -value, -value), blue, rt});

		uint32_t indices[] = {
			0, 2, 1, 1, 2, 3,	 // front
			4, 6, 5, 5, 6, 7,	 // right
			8, 10, 9, 9, 10, 11, // left

			12, 14, 13, 13, 14, 15, // back
			16, 18, 17, 17, 18, 19, // top
			20, 22, 21, 21, 22, 23, // bottom
		};
		vArray.indexCount = _countof(indices);
		vArray.PushIndices(indices, vArray.indexCount);
		auto vArraySize = vArray.vertices.size() * sizeof(Vertex);
		vArray.vertexBuffer = CreateBuffer(vArraySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vArray.indexBuffer = CreateBuffer(vArray.indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		// Write vertex data
		{
			void *p;
			vkMapMemory(m_base->GetVkDevice(), vArray.vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, vArray.vertices.data(), vArraySize);
			vkUnmapMemory(m_base->GetVkDevice(), vArray.vertexBuffer.memory);
		}
		// Write index data

		{
			void *p;
			vkMapMemory(m_base->GetVkDevice(), vArray.indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, indices, sizeof(indices));
			vkUnmapMemory(m_base->GetVkDevice(), vArray.indexBuffer.memory);
		}
		m_VertexArrays.emplace(std::pair<std::string, VertexArrayForVK>("BOX", vArray));
	}

	void VKRenderer::createSpriteVertices()
	{
		const float value = 1.f;
		const Vector2 lb(0.0f, 0.0f);
		const Vector2 lt(0.f, 1.f);
		const Vector2 rb(1.0f, 0.0f);
		const Vector2 rt(1.0f, 1.0f);
		std::array<float, 3> norm = {1, 1, 1};

		VertexArrayForVK vArray;
		vArray.vertices.push_back({Vector3(-value, value, value), norm, lb});
		vArray.vertices.push_back({Vector3(-value, -value, value), norm, lt});
		vArray.vertices.push_back({Vector3(value, value, value), norm, rb});
		vArray.vertices.push_back({Vector3(value, -value, value), norm, rt});

		uint32_t indices[] = {
			0, 2, 1, 1, 2, 3, // front
		};
		vArray.indexCount = _countof(indices);
		vArray.PushIndices(indices, vArray.indexCount);
		auto vArraySize = vArray.vertices.size() * sizeof(Vertex);
		vArray.vertexBuffer = CreateBuffer(vArraySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vArray.indexBuffer = CreateBuffer(vArray.indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		// Write vertex data
		{
			void *p;
			vkMapMemory(m_base->GetVkDevice(), vArray.vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, vArray.vertices.data(), vArraySize);
			vkUnmapMemory(m_base->GetVkDevice(), vArray.vertexBuffer.memory);
		}
		// Write index data

		{
			void *p;
			vkMapMemory(m_base->GetVkDevice(), vArray.indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, indices, sizeof(indices));
			vkUnmapMemory(m_base->GetVkDevice(), vArray.indexBuffer.memory);
		}
		m_VertexArrays.insert(std::pair<std::string, VertexArrayForVK>("SPRITE", vArray));
	}

	void VKRenderer::prepareImGUI()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForVulkan(m_base->m_window);

		uint32_t imageCount = m_base->mSwapchain->GetImageCount();
		ImGui_ImplVulkan_InitInfo info{};
		info.Instance = m_base->m_instance;
		info.PhysicalDevice = m_base->m_physDev;
		info.Device = m_base->GetVkDevice();
		info.QueueFamily = m_base->m_graphicsQueueIndex;
		info.Queue = m_base->GetVkQueue();
		info.DescriptorPool = m_descriptorPool;
		info.MinImageCount = 2;
		info.ImageCount = imageCount;
		if (!ImGui_ImplVulkan_Init(&info, m_base->m_renderPass))
		{
			std::exit(1);
		}

		// フォントテクスチャを転送する.
		VkCommandBufferAllocateInfo commandAI{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr, m_base->m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		VkCommandBuffer command;
		vkAllocateCommandBuffers(m_base->GetVkDevice(), &commandAI, &command);
		vkBeginCommandBuffer(command, &beginInfo);
		ImGui_ImplVulkan_CreateFontsTexture(command);
		vkEndCommandBuffer(command);

		VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr};
		submitInfo.pCommandBuffers = &command;
		submitInfo.commandBufferCount = 1;
		vkQueueSubmit(m_base->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		// フォントテクスチャ転送の完了を待つ.
		vkDeviceWaitIdle(m_base->GetVkDevice());
		vkFreeCommandBuffers(m_base->GetVkDevice(), m_base->m_commandPool, 1, &command);
	}
	void VKRenderer::renderImGUI(VkCommandBuffer command)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_base->m_window);
		ImGui::NewFrame();

		// ImGui ウィジェットを描画する.
		ImGui::Begin("Engine Info");
		ImGui::Text("%.1f fps", ImGui::GetIO().Framerate);
		if (ImGui::Button("toggleAPI"))
		{
			std::ofstream ofs("./api");
			ofs << "OpenGL";
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(
			ImGui::GetDrawData(), command);
	}

	void VKRenderer::prepareUniformBuffers()
	{

		m_instanceUniforms.resize(maxInstanceCount);
		for (auto &v : m_instanceUniforms)
		{
			VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			v = CreateBuffer(sizeof(InstanceData) * m_instanceUniforms.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
		}
		for (auto &s : mTextures3D)
		{
			s->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
			for (auto &v : s->uniformBuffers)
			{
				VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				v = CreateBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
			}
		}
		for (auto &s : mTextures2D)
		{
			s->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
			for (auto &v : s->uniformBuffers)
			{
				VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				v = CreateBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
			}
		}
	}
	void VKRenderer::prepareDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayoutBinding bindingUBO{}, bindingTex{}, bindingInstance{};
		bindingUBO.binding = 0;
		bindingUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingUBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindingUBO.descriptorCount = 1;
		bindings.push_back(bindingUBO);

		bindingTex.binding = 1;
		bindingTex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindingTex.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindingTex.descriptorCount = 1;
		bindings.push_back(bindingTex);

		bindingInstance.binding = 2;
		bindingInstance.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindingInstance.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindingInstance.descriptorCount = 1;
		bindings.push_back(bindingInstance);

		VkDescriptorSetLayoutCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.bindingCount = uint32_t(bindings.size());
		ci.pBindings = bindings.data();
		vkCreateDescriptorSetLayout(m_base->GetVkDevice(), &ci, nullptr, &m_descriptorSetLayout);
	}

	void VKRenderer::prepareDescriptorPool()
	{

		VkResult result;
		VkDescriptorPoolSize poolSize[] = {
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxpoolSize},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxpoolSize},
		};
		VkDescriptorPoolCreateInfo descPoolCI{
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			maxpoolSize * _countof(poolSize), // maxSets
			_countof(poolSize),
			poolSize,
		};
		result = vkCreateDescriptorPool(m_base->GetVkDevice(), &descPoolCI, nullptr, &m_descriptorPool);
	}

	void VKRenderer::prepareDescriptorSetAll()
	{
		layouts.resize(m_base->mSwapchain->GetImageCount() + (mTextures3D.size() + mTextures2D.size()));
		for (auto &i : layouts)
		{
			i = m_descriptorSetLayout;
		}
		for (auto &sprite : mTextures3D)
		{
			prepareDescriptorSet(sprite);
		}
		for (auto &sprite : mTextures2D)
		{
			prepareDescriptorSet(sprite);
		}
	}
	void VKRenderer::prepareDescriptorSet(std::shared_ptr<SpriteVK> sprite)
	{
		VkDescriptorSetAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = m_descriptorPool;
		ai.descriptorSetCount = uint32_t(sprite->uniformBuffers.size());
		ai.pSetLayouts = layouts.data();
		sprite->descripterSet.resize(sprite->uniformBuffers.size());
		vkAllocateDescriptorSets(m_base->GetVkDevice(), &ai, sprite->descripterSet.data());
		static int count = 0;
		// ディスクリプタセットへ書き込み.
		for (int i = 0; i < int(sprite->uniformBuffers.size()); ++i)
		{
			VkDescriptorBufferInfo descUBO{};
			descUBO.buffer = sprite->uniformBuffers[i].buffer;
			descUBO.offset = 0;
			descUBO.range = VK_WHOLE_SIZE;

			VkDescriptorImageInfo descImage{};
			descImage.imageView = mImageObjects[sprite->imageID].view;
			descImage.sampler = m_sampler;
			descImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorBufferInfo descInstance{};
			descInstance.buffer = m_instanceUniforms[0].buffer;
			descInstance.offset = 0;
			descInstance.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet ubo{};
			ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ubo.dstBinding = 0;
			ubo.descriptorCount = 1;
			ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ubo.pBufferInfo = &descUBO;
			ubo.dstSet = sprite->descripterSet[i];

			VkWriteDescriptorSet tex{};
			tex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			tex.dstBinding = 1;
			tex.descriptorCount = 1;
			tex.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			tex.pImageInfo = &descImage;
			tex.dstSet = sprite->descripterSet[i];

			VkWriteDescriptorSet ins{};
			ins.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ins.dstBinding = 2;
			ins.descriptorCount = 1;
			ins.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ins.pBufferInfo = &descInstance;
			ins.dstSet = sprite->descripterSet[i];

			std::vector<VkWriteDescriptorSet> writeSets =
				{
					ubo, tex, ins};
			vkUpdateDescriptorSets(m_base->GetVkDevice(), uint32_t(writeSets.size()), writeSets.data(), 0, nullptr);
			count++;
		}
	}
	BufferObject VKRenderer::CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
	{
		BufferObject obj;
		VkBufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.usage = usage;
		ci.size = size;
		auto result = vkCreateBuffer(m_base->GetVkDevice(), &ci, nullptr, &obj.buffer);
		m_base->checkResult(result);

		// メモリ量の算出
		VkMemoryRequirements reqs;
		vkGetBufferMemoryRequirements(m_base->GetVkDevice(), obj.buffer, &reqs);
		VkMemoryAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = reqs.size;
		// メモリタイプの判定
		info.memoryTypeIndex = m_base->getMemoryTypeIndex(reqs.memoryTypeBits, flags);
		// メモリの確保
		vkAllocateMemory(m_base->GetVkDevice(), &info, nullptr, &obj.memory);

		// メモリのバインド
		vkBindBufferMemory(m_base->GetVkDevice(), obj.buffer, obj.memory, 0);
		return obj;
	}
	BufferObject VKRenderer::CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, const void *initialData)
	{
		BufferObject obj;
		VkBufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.usage = usage;
		ci.size = size;
		auto result = vkCreateBuffer(m_base->GetVkDevice(), &ci, nullptr, &obj.buffer);
		m_base->checkResult(result);

		// メモリ量の算出
		VkMemoryRequirements reqs;
		vkGetBufferMemoryRequirements(m_base->GetVkDevice(), obj.buffer, &reqs);
		VkMemoryAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = reqs.size;
		// メモリタイプの判定
		info.memoryTypeIndex = m_base->getMemoryTypeIndex(reqs.memoryTypeBits, flags);
		// メモリの確保
		vkAllocateMemory(m_base->GetVkDevice(), &info, nullptr, &obj.memory);

		// メモリのバインド
		vkBindBufferMemory(m_base->GetVkDevice(), obj.buffer, obj.memory, 0);

		if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 &&
			initialData != nullptr)
		{
			void *p;
			vkMapMemory(m_base->GetVkDevice(), obj.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, initialData, size);
			vkUnmapMemory(m_base->GetVkDevice(), obj.memory);
		}
		return obj;
	}

	VkSampler VKRenderer::createSampler()
	{
		VkSampler sampler;
		VkSamplerCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		ci.minFilter = VK_FILTER_LINEAR;
		ci.magFilter = VK_FILTER_LINEAR;
		ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		ci.maxAnisotropy = 1.0f;
		ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		vkCreateSampler(m_base->GetVkDevice(), &ci, nullptr, &sampler);
		return sampler;
	}

	ImageObject VKRenderer::createTextureFromSurface(const ::SDL_Surface &surface)
	{
		::SDL_Surface surf = surface;
		::SDL_LockSurface(&surf);
		auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
		formatbuf->BytesPerPixel = 4;
		auto imagedata = ::SDL_ConvertSurface(&surf, formatbuf, 0);
		::SDL_UnlockSurface(&surf);
		BufferObject stagingBuffer;
		ImageObject texture{};
		int width = imagedata->w, height = imagedata->h;
		auto *pImage = imagedata->pixels;
		auto format = VK_FORMAT_R8G8B8A8_UNORM;

		{
			// Create VkImage texture
			VkImageCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ci.extent = {uint32_t(width), uint32_t(height), 1};
			ci.format = format;
			ci.imageType = VK_IMAGE_TYPE_2D;
			ci.arrayLayers = 1;
			ci.mipLevels = 1;
			ci.samples = VK_SAMPLE_COUNT_1_BIT;
			ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			ci.tiling = VkImageTiling::VK_IMAGE_TILING_LINEAR;
			ci.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			vkCreateImage(m_base->GetVkDevice(), &ci, nullptr, &texture.image);

			VkMemoryRequirements reqs;
			vkGetImageMemoryRequirements(m_base->GetVkDevice(), texture.image, &reqs);
			VkMemoryAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			info.allocationSize = reqs.size;
			// Judge memory type
			info.memoryTypeIndex = m_base->getMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			// Allocate memory
			vkAllocateMemory(m_base->GetVkDevice(), &info, nullptr, &texture.memory);
			// Bind memory
			vkBindImageMemory(m_base->GetVkDevice(), texture.image, texture.memory, 0);
		}

		{
			uint32_t imageSize = imagedata->h * imagedata->w * 4;
			stagingBuffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			void *p;
			vkMapMemory(m_base->GetVkDevice(), stagingBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
			memcpy(p, pImage, imageSize);
			vkUnmapMemory(m_base->GetVkDevice(), stagingBuffer.memory);
		}

		VkBufferImageCopy copyRegion{};
		copyRegion.imageExtent = {uint32_t(width), uint32_t(height), 1};
		copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		VkCommandBuffer command;
		{
			VkCommandBufferAllocateInfo ai{};
			ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			ai.commandBufferCount = 1;
			ai.commandPool = m_base->m_commandPool;
			ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			vkAllocateCommandBuffers(m_base->GetVkDevice(), &ai, &command);
		}

		VkCommandBufferBeginInfo commandBI{};
		commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(command, &commandBI);
		setImageMemoryBarrier(command, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(command, stagingBuffer.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		setImageMemoryBarrier(command, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkEndCommandBuffer(command);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command;
		vkQueueSubmit(m_base->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		{
			// Create view for texture reference
			VkImageViewCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ci.image = texture.image;
			ci.format = format;
			ci.components = {
				.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,
				.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,
				.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B,
				.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A,
			};

			ci.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			vkCreateImageView(m_base->GetVkDevice(), &ci, nullptr, &texture.view);
		}

		vkDeviceWaitIdle(m_base->GetVkDevice());
		vkFreeCommandBuffers(m_base->GetVkDevice(), m_base->m_commandPool, 1, &command);

		vkFreeMemory(m_base->GetVkDevice(), stagingBuffer.memory, nullptr);
		vkDestroyBuffer(m_base->GetVkDevice(), stagingBuffer.buffer, nullptr);

		return texture;
	}
	ImageObject VKRenderer::createTextureFromMemory(const std::vector<char> &imageData)
	{
		auto *rw = ::SDL_RWFromMem((void *)imageData.data(), imageData.size());
		::SDL_Surface *surface;
		surface = ::IMG_Load_RW(rw, 1);
		::SDL_LockSurface(surface);
		auto formatbuf = ::SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
		formatbuf->BytesPerPixel = 4;
		auto imagedata = SDL_ConvertSurface(surface, formatbuf, 0);
		SDL_UnlockSurface(surface);
		BufferObject stagingBuffer;
		ImageObject texture{};
		int width = imagedata->w, height = imagedata->h;
		auto *pImage = imagedata->pixels;
		auto format = VK_FORMAT_R8G8B8A8_UNORM;

		{
			// テクスチャのVkImage を生成
			VkImageCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			ci.extent = {uint32_t(width), uint32_t(height), 1};
			ci.format = format;
			ci.imageType = VK_IMAGE_TYPE_2D;
			ci.arrayLayers = 1;
			ci.mipLevels = 1;
			ci.samples = VK_SAMPLE_COUNT_1_BIT;
			ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			vkCreateImage(m_base->GetVkDevice(), &ci, nullptr, &texture.image);

			// メモリ量の算出
			VkMemoryRequirements reqs;
			vkGetImageMemoryRequirements(m_base->GetVkDevice(), texture.image, &reqs);
			VkMemoryAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			info.allocationSize = reqs.size;
			// メモリタイプの判定
			info.memoryTypeIndex = m_base->getMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			// メモリの確保
			vkAllocateMemory(m_base->GetVkDevice(), &info, nullptr, &texture.memory);
			// メモリのバインド
			vkBindImageMemory(m_base->GetVkDevice(), texture.image, texture.memory, 0);
		}

		{
			uint32_t imageSize = width * height * sizeof(uint32_t);
			// ステージングバッファを用意.
			stagingBuffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pImage);
		}

		VkBufferImageCopy copyRegion{};
		copyRegion.imageExtent = {uint32_t(width), uint32_t(height), 1};
		copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		VkCommandBuffer command;
		{
			VkCommandBufferAllocateInfo ai{};
			ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			ai.commandBufferCount = 1;
			ai.commandPool = m_base->m_commandPool;
			ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			vkAllocateCommandBuffers(m_base->GetVkDevice(), &ai, &command);
		}

		VkCommandBufferBeginInfo commandBI{};
		commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(command, &commandBI);
		setImageMemoryBarrier(command, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(command, stagingBuffer.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		setImageMemoryBarrier(command, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vkEndCommandBuffer(command);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command;
		vkQueueSubmit(m_base->GetVkQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		{
			// テクスチャ参照用のビューを生成
			VkImageViewCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ci.image = texture.image;
			ci.format = format;
			ci.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A,
			};
			ci.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			vkCreateImageView(m_base->GetVkDevice(), &ci, nullptr, &texture.view);
		}

		vkDeviceWaitIdle(m_base->GetVkDevice());
		vkFreeCommandBuffers(m_base->GetVkDevice(), m_base->m_commandPool, 1, &command);

		// ステージングバッファ解放.
		vkFreeMemory(m_base->GetVkDevice(), stagingBuffer.memory, nullptr);
		vkDestroyBuffer(m_base->GetVkDevice(), stagingBuffer.buffer, nullptr);

		return texture;
	}

	void VKRenderer::setImageMemoryBarrier(
		VkCommandBuffer command,
		VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier imb{};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = oldLayout;
		imb.newLayout = newLayout;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		imb.image = image;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imb.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}

		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			imb.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}

		srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;	 // パイプライン中でリソースへの書込み最終のステージ.
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // パイプライン中で次にリソースに書き込むステージ.

		vkCmdPipelineBarrier(
			command,
			srcStage,
			dstStage,
			0,
			0, // memoryBarrierCount
			nullptr,
			0, // bufferMemoryBarrierCount
			nullptr,
			1, // imageMemoryBarrierCount
			&imb);
	}

	uint32_t VKRenderer::GetMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps) const
	{
		uint32_t result = ~0u;
		for (uint32_t i = 0; i < m_physicalMemProps.memoryTypeCount; ++i)
		{
			if (requestBits & 1)
			{
				const auto &types = m_physicalMemProps.memoryTypes[i];
				if ((types.propertyFlags & requestProps) == requestProps)
				{
					result = i;
					break;
				}
			}
			requestBits >>= 1;
		}
		return result;
	}

	void VKRenderer::RegisterRenderPass(const std::string &name, VkRenderPass renderPass)
	{
		if (m_base)
			m_base->m_renderPass;
	}

	ImageObject VKRenderer::CreateTexture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
	{
		ImageObject obj;
		VkImageCreateInfo imageCI{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			nullptr,
			0,
			VK_IMAGE_TYPE_2D,
			format,
			{width, height, 1},
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			VK_IMAGE_LAYOUT_UNDEFINED};
		auto result = vkCreateImage(m_base->GetVkDevice(), &imageCI, nullptr, &obj.image);
		ThrowIfFailed(result, "vkCreateImage Failed.");

		VkMemoryRequirements reqs;
		vkGetImageMemoryRequirements(m_base->GetVkDevice(), obj.image, &reqs);
		VkMemoryAllocateInfo info{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			reqs.size,
			GetMemoryTypeIndex(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
		result = vkAllocateMemory(m_base->GetVkDevice(), &info, nullptr, &obj.memory);
		ThrowIfFailed(result, "vkAllocateMemory Failed.");
		vkBindImageMemory(m_base->GetVkDevice(), obj.image, obj.memory, 0);

		VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			imageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		VkImageViewCreateInfo viewCI{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			obj.image,
			VK_IMAGE_VIEW_TYPE_2D,
			imageCI.format,
			vkutil::DefaultComponentMapping(),
			{imageAspect, 0, 1, 0, 1}};
		result = vkCreateImageView(m_base->GetVkDevice(), &viewCI, nullptr, &obj.view);
		ThrowIfFailed(result, "vkCreateImageView Failed.");
		return obj;
	}

	VkFramebuffer VKRenderer::CreateFramebuffer(VkRenderPass renderPass, uint32_t width, uint32_t height, uint32_t viewCount, VkImageView *views)
	{
		VkFramebufferCreateInfo fbCI{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			renderPass,
			viewCount,
			views,
			width,
			height,
			1,
		};
		VkFramebuffer framebuffer;
		auto result = vkCreateFramebuffer(m_base->GetVkDevice(), &fbCI, nullptr, &framebuffer);
		ThrowIfFailed(result, "vkCreateFramebuffer Failed.");
		return framebuffer;
	}

	void VKRenderer::DestroyBuffer(BufferObject bufferObj)
	{
		vkDestroyBuffer(m_base->GetVkDevice(), bufferObj.buffer, nullptr);
		vkFreeMemory(m_base->GetVkDevice(), bufferObj.memory, nullptr);
	}

	void VKRenderer::DestroyImage(ImageObject imageObj)
	{
		vkDestroyImage(m_base->GetVkDevice(), imageObj.image, nullptr);
		vkFreeMemory(m_base->GetVkDevice(), imageObj.memory, nullptr);
		if (imageObj.view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_base->GetVkDevice(), imageObj.view, nullptr);
		}
	}

	void VKRenderer::DestroyFramebuffers(uint32_t count, VkFramebuffer *framebuffers)
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			vkDestroyFramebuffer(m_base->GetVkDevice(), framebuffers[i], nullptr);
		}
	}

	VkCommandBuffer VKRenderer::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandAI{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr, m_base->m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1};
		VkCommandBufferBeginInfo beginInfo{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};

		VkCommandBuffer command;
		vkAllocateCommandBuffers(m_base->GetVkDevice(), &commandAI, &command);
		vkBeginCommandBuffer(command, &beginInfo);
		return command;
	}

	void VKRenderer::FinishCommandBuffer(VkCommandBuffer command)
	{
		auto result = vkEndCommandBuffer(command);
		ThrowIfFailed(result, "vkEndCommandBuffer Failed.");
		VkFence fence;
		VkFenceCreateInfo fenceCI{
			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			nullptr, 0};
		result = vkCreateFence(m_base->GetVkDevice(), &fenceCI, nullptr, &fence);

		VkSubmitInfo submitInfo{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			0,
			nullptr,
			nullptr,
			1,
			&command,
			0,
			nullptr,
		};
		vkQueueSubmit(m_base->GetVkQueue(), 1, &submitInfo, fence);
		vkWaitForFences(m_base->GetVkDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(m_base->GetVkDevice(), fence, nullptr);
	}

	std::vector<BufferObject> VKRenderer::CreateUniformBuffers(uint32_t size, uint32_t imageCount)
	{
		std::vector<BufferObject> buffers(imageCount);
		for (auto &b : buffers)
		{
			VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			b = CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, props);
		}
		return buffers;
	}

	void VKRenderer::WriteToHostVisibleMemory(VkDeviceMemory memory, uint32_t size, const void *pData)
	{
		void *p;
		vkMapMemory(m_base->GetVkDevice(), memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, pData, size);
		vkUnmapMemory(m_base->GetVkDevice(), memory);
	}

	void VKRenderer::AllocateCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands)
	{
		VkCommandBufferAllocateInfo commandAI{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr, m_base->m_commandPool,
			VK_COMMAND_BUFFER_LEVEL_SECONDARY, count};
		auto result = vkAllocateCommandBuffers(m_base->GetVkDevice(), &commandAI, pCommands);
		ThrowIfFailed(result, "vkAllocateCommandBuffers Failed.");
	}

	void VKRenderer::FreeCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands)
	{
		vkFreeCommandBuffers(m_base->GetVkDevice(), m_base->m_commandPool, count, pCommands);
	}

	void VKRenderer::TransferStageBufferToImage(const BufferObject &srcBuffer, const ImageObject &dstImage, const VkBufferImageCopy *region)
	{
		VkImageMemoryBarrier imb{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, dstImage.image, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

		auto command = CreateCommandBuffer();
		vkCmdPipelineBarrier(command,
							 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
							 0, 0, nullptr,
							 0, nullptr, 1, &imb);

		vkCmdCopyBufferToImage(
			command,
			srcBuffer.buffer, dstImage.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, region);
		imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkCmdPipelineBarrier(command,
							 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							 0, 0, nullptr,
							 0, nullptr,
							 1, &imb);
		FinishCommandBuffer(command);
	}

	void VKRenderer::registerImageObject(std::shared_ptr<Texture> texture)
	{
		if (!mImageObjects.contains(texture->id))
		{
			mImageObjects.insert({texture->id, VKRenderer::createTextureFromSurface(texture->GetSurface())});
		}
	}
	VkRenderPass VKRenderer::GetRenderPass(const std::string &name)
	{
		if (m_base)
			return m_base->m_renderPass;
		return 0;
	}
	VkDevice VKRenderer::GetDevice()
	{
		if (m_base)
			return m_base->GetVkDevice();
		return m_base->GetVkDevice();
	}
	void VKRenderer::registerTexture(std::shared_ptr<SpriteVK> texture, std::string_view ID, TextureType type)
	{
		if (TextureType::Image3D == type)
		{
			mTextures3D.push_back(texture);
			mTextures3D.back()->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
			for (auto &v : mTextures3D.back()->uniformBuffers)
			{
				VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				v = CreateBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
			}
			texture->imageID = ID.data();
			layouts.push_back(m_descriptorSetLayout);
			prepareDescriptorSet(texture);
		}
		else
		{
			auto iter = mTextures2D.begin();
			for (;
				 iter != mTextures2D.end();
				 ++iter)
			{
				if (texture->sprite->drawOrder < (*iter)->sprite->drawOrder)
				{
					break;
				}
			}

			// Inserts element before position of iterator
			mTextures2D.insert(iter, texture);
			texture->uniformBuffers.resize(m_base->mSwapchain->GetImageCount());
			for (auto &v : texture->uniformBuffers)
			{
				VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				v = CreateBuffer(sizeof(ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
			}
			texture->imageID = ID.data();
			layouts.push_back(m_descriptorSetLayout);
			prepareDescriptorSet(texture);
		}
	}
	void VKRenderer::unregisterTexture(std::shared_ptr<SpriteVK> texture, TextureType type)
	{
		if (TextureType::Image3D == type)
		{
			auto itr = std::find(mTextures3D.begin(), mTextures3D.end(), texture);
			if (itr != mTextures3D.end())
			{
				DestroyBuffer((*itr)->buffer);
				vkFreeDescriptorSets(m_base->GetVkDevice(), m_descriptorPool, static_cast<uint32_t>(texture->uniformBuffers.size()), texture->descripterSet.data());
				for (auto &i : (*itr)->uniformBuffers)
				{
					DestroyBuffer(i);
				}
				mTextures3D.erase(itr);
				mTextures3D.shrink_to_fit();
			}
		}
		else
		{
			auto device = m_base->GetVkDevice();
			auto itr = std::find(mTextures2D.begin(), mTextures2D.end(), texture);
			DestroyVulkanObject<VkBuffer>(device, (*itr)->buffer.buffer, &vkDestroyBuffer);
			DestroyVulkanObject<VkDeviceMemory>(device, (*itr)->buffer.memory, &vkFreeMemory);
			vkFreeDescriptorSets(device, m_descriptorPool, static_cast<uint32_t>(texture->uniformBuffers.size()), texture->descripterSet.data());
			for (auto &i : (*itr)->uniformBuffers)
			{
				DestroyVulkanObject<VkBuffer>(device, i.buffer, &vkDestroyBuffer);
				DestroyVulkanObject<VkDeviceMemory>(device, i.memory, &vkFreeMemory);
			}
			if (itr != mTextures2D.end())
			{
				mTextures2D.erase(itr);
			}
		}
	}
	void VKRenderer::renderEffekseer(VkCommandBuffer command)
	{
		mEffectManager->GetMemoryPool()->NewFrame();
		EffekseerRendererVulkan::BeginCommandList(mEffectManager->GetCommandList(), command);
		mEffectManager->GetRenderer()->SetCommandList(mEffectManager->GetCommandList());
		static float oldtime = SDL_GetTicks();
		float nowtime = SDL_GetTicks();
		static float timer = 0.f;
		timer += (nowtime - oldtime) / 1000.f;
		for (auto i : this->mRenderer->GetEffects())
		{
			auto eref = mEffectManager->GetEffect(i->GetPath());
			auto p = i->GetPosition();
			if (timer > 0.2f)
			{
				mEffectManager->GetManager()->StopEffect(i->handle);
				i->handle = mEffectManager->GetManager()->Play(eref, p.x, p.y, p.z);
				timer = 0.f;
			}
			mEffectManager->GetManager()->SetLocation(i->handle, ::Effekseer::Vector3D(p.x, p.y, p.z));
		}
		oldtime = SDL_GetTicks();
		mEffectManager->GetManager()->Update();
		mEffectManager->GetRenderer()->BeginRendering();
		mEffectManager->GetManager()->Draw();
		mEffectManager->GetRenderer()->EndRendering();
		mEffectManager->GetRenderer()->SetCommandList(mEffectManager->GetCommandList());
		EffekseerRendererVulkan::EndCommandList(mEffectManager->GetCommandList());
	}
}
#endif