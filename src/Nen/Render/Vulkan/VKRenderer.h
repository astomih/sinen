#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <Nen.hpp>
#include "VKBase.h"
#include <unordered_map>
#include "Pipeline.h"
#include "PipelineLayout.h"
#include <array>
#endif

namespace nen
{
	class Renderer;
}

#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <DrawObject/DrawObject.hpp>
namespace nen::vk
{
	struct BufferObject
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
	struct ImageObject
	{
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	class SpriteVK
	{
	public:
		std::vector<VkDescriptorSet> descripterSet;
		std::vector<BufferObject> uniformBuffers;
		std::string imageID;
		std::shared_ptr<Texture> mTexture;
		BufferObject buffer;
		bool isInstance = false;
		std::shared_ptr<nen::DrawObject> sprite;
	};

	struct VertexArrayForVK : public VertexArray
	{
		BufferObject vertexBuffer;
		BufferObject indexBuffer;
	};

	struct InstanceData
	{
		Matrix4 world;
	};

	class VKRenderer : public IRenderer
	{
	public:
		VKRenderer();
		~VKRenderer() override
		{
		}
		void Initialize(struct SDL_Window *window) override;
		void Shutdown() override;
		void Render() override;
		void AddVertexArray(const VertexArray &vArray, std::string_view name) override;
		void ChangeBufferDrawObject(std::shared_ptr<class DrawObject> sprite, const TextureType type) override;

		void AddDrawObject2D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture) override;
		void RemoveDrawObject2D(std::shared_ptr<class DrawObject> sprite) override;

		void AddDrawObject3D(std::shared_ptr<class DrawObject> sprite, std::shared_ptr<Texture> texture) override;
		void RemoveDrawObject3D(std::shared_ptr<class DrawObject> sprite) override;

		void AddGUI(std::shared_ptr<class UIScreen> ui) override;
		void RemoveGUI(std::shared_ptr<class UIScreen> ui) override;

		nen::Renderer *GetRenderer() { return mRenderer; }
		void prepare();
		void cleanup();
		void makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri, VkCommandBufferBeginInfo &ci, VkFence &fence);
		void draw3d(VkCommandBuffer);
		void draw2d(VkCommandBuffer);
		void drawGUI(VkCommandBuffer);
		BufferObject CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VKBase *GetBase() { return m_base.get(); }
		BufferObject CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, const void *initialData);
		void registerTexture(std::shared_ptr<SpriteVK> texture, std::string_view, TextureType type);
		void unregisterTexture(std::shared_ptr<SpriteVK> texture, TextureType type);
		void registerImageObject(std::shared_ptr<Texture>);
		VkPipelineLayout GetPipelineLayout(const std::string &name) { return mPipelineLayout.GetLayout(); }
		VkDescriptorSetLayout GetDescriptorSetLayout(const std::string &name) { return m_descriptorSetLayout; }
		VkRenderPass GetRenderPass(const std::string &name);
		VkDescriptorPool GetDescriptorPool() const { return m_descriptorPool; }
		VkDevice GetDevice();
		uint32_t GetMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps) const;
		void RegisterLayout(const std::string &name, VkDescriptorSetLayout &layout) { m_descriptorSetLayout; }
		void RegisterRenderPass(const std::string &name, VkRenderPass renderPass);
		ImageObject CreateTexture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, uint32_t width, uint32_t height, uint32_t viewCount, VkImageView *views);
		void DestroyBuffer(BufferObject bufferObj);
		void DestroyImage(ImageObject imageObj);
		void DestroyFramebuffers(uint32_t count, VkFramebuffer *framebuffers);
		VkCommandBuffer CreateCommandBuffer();
		void FinishCommandBuffer(VkCommandBuffer command);
		std::vector<BufferObject> CreateUniformBuffers(uint32_t size, uint32_t imageCount);
		const std::vector<std::shared_ptr<SpriteVK>> &GetSprite2Ds() { return mTextures2D; }
		const std::vector<std::shared_ptr<SpriteVK>> &GetSprite3Ds() { return mTextures3D; }
		void WriteToHostVisibleMemory(VkDeviceMemory memory, uint32_t size, const void *pData);
		void AllocateCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands);
		void FreeCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands);
		void TransferStageBufferToImage(const BufferObject &srcBuffer, const ImageObject &dstImage, const VkBufferImageCopy *region);
		void MapMemory(VkDeviceMemory memory, void *data, size_t size);
		void UpdateVertexArray(const VertexArrayForVK &vArray, std::string_view name);

	private:
		std::unique_ptr<class VKBase> m_base;
		std::unique_ptr<class EffectManagerVK> mEffectManager;
		void createBoxVertices();
		void createSpriteVertices();
		void prepareUniformBuffers();
		void prepareDescriptorSetLayout();
		void prepareDescriptorPool();
		void prepareDescriptorSetAll();
		void prepareDescriptorSet(std::shared_ptr<SpriteVK>);
		void prepareImGUI();
		void renderImGUI(VkCommandBuffer command);
		void renderEffekseer(VkCommandBuffer command);
		VkSampler createSampler();
		ImageObject createTextureFromSurface(const ::SDL_Surface &surface);
		ImageObject createTextureFromMemory(const std::vector<char> &imageData);
		void setImageMemoryBarrier(VkCommandBuffer command, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
		std::unordered_map<std::string, VertexArrayForVK> m_VertexArrays;
		ImageObject m_shadowColor;
		ImageObject m_shadowDepth;
		std::vector<float> m_faceWeights;
		VkFramebuffer m_shadowFramebuffer;
		std::vector<VkDescriptorSetLayout> layouts;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkDescriptorPool m_descriptorPool;
		VkSampler m_sampler;
		VkPhysicalDeviceMemoryProperties m_physicalMemProps;
		PipelineLayout mPipelineLayout;
		Pipeline pipelineOpaque;
		Pipeline pipelineAlpha;
		Pipeline pipeline2D;
		std::vector<std::shared_ptr<SpriteVK>> mTextures3D;
		std::vector<std::shared_ptr<SpriteVK>> mTextures2D;
		std::unordered_map<std::string, ImageObject> mImageObjects;
		std::vector<InstanceData> instance;
		std::vector<BufferObject> m_instanceUniforms;
		std::vector<std::shared_ptr<class UIScreen>> mGUI;
		int instanceCount;
	};
}
#endif