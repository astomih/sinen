#pragma once
#ifndef EMSCRIPTEN

#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_sampler.hpp>
#include <gpu/gpu_texture.hpp>

#include "volk.hpp"

namespace sinen::gpu::vulkan::convert {
VkFormat textureFormatFrom(gpu::TextureFormat format);
VkImageAspectFlags aspectMaskFrom(VkFormat format);
VkSampleCountFlagBits sampleCountFrom(gpu::SampleCount sampleCount);

VkFilter filterFrom(gpu::Filter filter);
VkSamplerMipmapMode mipmapModeFrom(gpu::MipmapMode mode);
VkSamplerAddressMode addressModeFrom(gpu::AddressMode mode);
VkCompareOp compareOpFrom(gpu::CompareOp op);

VkPrimitiveTopology primitiveTopologyFrom(gpu::PrimitiveType primitiveType);
VkPolygonMode polygonModeFrom(gpu::FillMode fillMode);
VkCullModeFlags cullModeFrom(gpu::CullMode cullMode);
VkFrontFace frontFaceFrom(gpu::FrontFace frontFace);

VkBlendFactor blendFactorFrom(gpu::BlendFactor factor);
VkBlendOp blendOpFrom(gpu::BlendOp op);

VkVertexInputRate vertexInputRateFrom(gpu::VertexInputRate rate);
VkFormat vertexElementFormatFrom(gpu::VertexElementFormat format);

VkAttachmentLoadOp loadOpFrom(gpu::LoadOp op);
VkAttachmentStoreOp storeOpFrom(gpu::StoreOp op);
} // namespace sinen::gpu::vulkan::convert

#endif // EMSCRIPTEN
