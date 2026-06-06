#ifndef SINEN_D3D12_CONVERT_HPP
#define SINEN_D3D12_CONVERT_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_render_pass.hpp>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace sinen::gpu::d3d12::convert {
D3D12_RESOURCE_STATES bufferStateFrom(BufferUsage usage);
D3D12_RESOURCE_FLAGS textureFlagsFrom(TextureUsage usage);
D3D12_RESOURCE_DIMENSION textureDimensionFrom(TextureType type);
DXGI_FORMAT textureFormatFrom(TextureFormat format);
TextureFormat textureFormatFrom(DXGI_FORMAT format);
D3D12_FILTER filterFrom(const Sampler::CreateInfo &createInfo);
D3D12_TEXTURE_ADDRESS_MODE addressModeFrom(AddressMode mode);
D3D12_PRIMITIVE_TOPOLOGY primitiveTopologyFrom(PrimitiveType type);
D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyTypeFrom(PrimitiveType type);
D3D12_FILL_MODE fillModeFrom(FillMode mode);
D3D12_CULL_MODE cullModeFrom(CullMode mode);
D3D12_COMPARISON_FUNC compareOpFrom(CompareOp op);
D3D12_STENCIL_OP stencilOpFrom(StencilOp op);
D3D12_BLEND blendFactorFrom(BlendFactor factor);
D3D12_BLEND_OP blendOpFrom(BlendOp op);
UINT8 colorWriteMaskFrom(UInt8 mask);
DXGI_FORMAT vertexFormatFrom(VertexElementFormat format);
UINT vertexFormatSize(VertexElementFormat format);
DXGI_FORMAT indexFormatFrom(IndexElementSize size);
UINT sampleCountFrom(SampleCount count);
} // namespace sinen::gpu::d3d12::convert

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_CONVERT_HPP
