#pragma once
#include <array>
#include "VKBase.h"
#include <Engine/include/Math.hpp>
#include <Engine/include/Texture.h>
#include <unordered_map>
enum class TextureType
{
	Image2D,
	Image3D,
	Font
};
class SpriteGL;
#include <Engine/include/OpenGL/GLRenderer.h>
class SpriteVK : public SpriteGL
{
public:
	std::vector<VkDescriptorSet> descripterSet;
	std::vector<BufferObject> uniformBuffers;
	std::string imageID;
	std::shared_ptr<Texture> mTexture;
	BufferObject buffer;
	bool isInstance = false;
};

struct InstanceData
{
	glm::mat4 world;
};

class VKRenderer
{
public:
	VKRenderer();

	void initialize(SDL_Window *window, const char *appName);
	void terminate();
	void prepare();
	void render();
	void cleanup();
	void makeCommand(VkCommandBuffer command, VkRenderPassBeginInfo &ri, VkCommandBufferBeginInfo &ci, VkFence &fence);
	void draw3d(VkCommandBuffer);
	void draw2d(VkCommandBuffer);
	BufferObject CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	BufferObject CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, const void *initialData);

	void registerTexture(std::shared_ptr<SpriteVK> &texture, std::string_view, TextureType type);
	void registerImageObject(std::shared_ptr<Texture>);
	VkPipelineLayout GetPipelineLayout(const std::string &name) { return m_pipelineLayout; }
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

	// ホストから見えるメモリ領域にデータを書き込む.以下バッファを対象に使用.
	// - ステージングバッファ
	// - ユニフォームバッファ
	void WriteToHostVisibleMemory(VkDeviceMemory memory, uint32_t size, const void *pData);

	void AllocateCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands);
	void FreeCommandBufferSecondary(uint32_t count, VkCommandBuffer *pCommands);

	void TransferStageBufferToImage(const BufferObject &srcBuffer, const ImageObject &dstImage, const VkBufferImageCopy *region);

private:
	std::unique_ptr<class VKBase> m_base;
	void makeSpriteGeometry();
	void prepareUniformBuffers();
	void prepareDescriptorSetLayout();
	void prepareDescriptorPool();
	void prepareDescriptorSetAll();
	void prepareDescriptorSet(std::shared_ptr<SpriteVK>);
	void prepareImGUI();
	void renderImGUI(VkCommandBuffer command);

	VkPipelineShaderStageCreateInfo loadShaderModule(const char *fileName, VkShaderStageFlagBits stage);
	VkSampler createSampler();
	ImageObject createTextureFromSurface(const SDL_Surface &surface);
	ImageObject createTextureFromMemory(const std::vector<char> &imageData);
	void setImageMemoryBarrier(VkCommandBuffer command, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	BufferObject m_vertexBuffer;
	BufferObject m_indexBuffer;

	ImageObject m_shadowColor;
	ImageObject m_shadowDepth;
	std::vector<float> m_faceWeights;
	VkFramebuffer m_shadowFramebuffer;
	std::vector<VkDescriptorSetLayout> layouts;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	VkSampler m_sampler;

	VkPhysicalDeviceMemoryProperties m_physicalMemProps;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipelineOpaque;
	VkPipeline m_pipelineAlpha;
	VkPipeline m_pipeline2D;
	uint32_t m_indexCount;
	std::vector<std::shared_ptr<SpriteVK>> mTextures3D;
	std::vector<std::shared_ptr<SpriteVK>> mTextures2D;
	std::unordered_map<std::string, ImageObject> mImageObjects;
	std::vector<InstanceData> instance;
	std::vector<BufferObject> m_instanceUniforms;
	int instanceCount;
};