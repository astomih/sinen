#ifndef SINEN_WEBGPU_CONVERT_HPP
#define SINEN_WEBGPU_CONVERT_HPP

#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_render_pass.hpp>
#include <gpu/gpu_sampler.hpp>
#include <gpu/gpu_texture.hpp>
#include <gpu/gpu_transfer_buffer.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu::convert {
WGPULoadOp LoadOpFrom(LoadOp loadOp);
WGPUStoreOp StoreOpFrom(StoreOp storeOp);
WGPUPrimitiveTopology PrimitiveTypeFrom(PrimitiveType primitiveType);
WGPUTextureFormat TextureFormatFrom(TextureFormat textureFormat);
TextureFormat TextureFormatTo(WGPUTextureFormat textureFormat);
WGPUTextureDimension TextureDimensionFrom(TextureType textureType);
WGPUTextureViewDimension TextureViewDimensionFrom(TextureType textureType);
WGPUTextureUsage TextureUsageFrom(TextureUsage textureUsage);
uint32_t SampleCountFrom(SampleCount sampleCount);
WGPUFilterMode FilterFrom(Filter filter);
WGPUMipmapFilterMode MipmapModeFrom(MipmapMode mipmapMode);
WGPUAddressMode AddressModeFrom(AddressMode addressMode);
WGPUCompareFunction CompareOpFrom(CompareOp compareOp);
WGPUStencilOperation StencilOpFrom(StencilOp op);
WGPUBufferUsage BufferUsageFrom(BufferUsage bufferUsage);
WGPUVertexFormat VertexElementFormatFrom(VertexElementFormat format);
WGPUVertexStepMode VertexInputRateFrom(VertexInputRate vertexInputRate);
WGPUCullMode CullModeFrom(CullMode cullMode);
WGPUFrontFace FrontFaceFrom(FrontFace frontFace);
WGPUBlendFactor BlendFactorFrom(BlendFactor blendFactor);
WGPUBlendOperation BlendOpFrom(BlendOp blendOp);
WGPUColorWriteMask ColorWriteMaskFrom(UInt8 mask);
WGPUIndexFormat IndexFormatFrom(IndexElementSize size);
WGPUOptionalBool OptionalBoolFrom(bool value);
WGPUMapMode MapModeFrom(TransferBufferUsage usage);
UInt32 bytesPerPixel(TextureFormat textureFormat);
} // namespace sinen::gpu::webgpu::convert

#endif // SINEN_WEBGPU_CONVERT_HPP