#include "d3d12_convert.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

namespace sinen::gpu::d3d12::convert {
D3D12_RESOURCE_STATES bufferStateFrom(BufferUsage usage) {
  switch (usage) {
  case BufferUsage::Vertex:
    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  case BufferUsage::Index:
    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
  case BufferUsage::Indirect:
    return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
  }
  return D3D12_RESOURCE_STATE_COMMON;
}

D3D12_RESOURCE_FLAGS textureFlagsFrom(TextureUsage usage) {
  switch (usage) {
  case TextureUsage::Sampler:
    return D3D12_RESOURCE_FLAG_NONE;
  case TextureUsage::ColorTarget:
    return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  case TextureUsage::DepthStencilTarget:
    return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }
  return D3D12_RESOURCE_FLAG_NONE;
}

D3D12_RESOURCE_DIMENSION textureDimensionFrom(TextureType type) {
  switch (type) {
  case TextureType::Texture3D:
    return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
  default:
    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  }
}

DXGI_FORMAT textureFormatFrom(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8_UNORM:
    return DXGI_FORMAT_R8_UNORM;
  case TextureFormat::R8G8_UNORM:
    return DXGI_FORMAT_R8G8_UNORM;
  case TextureFormat::R8G8B8A8_UNORM:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case TextureFormat::B8G8R8A8_UNORM:
    return DXGI_FORMAT_B8G8R8A8_UNORM;
  case TextureFormat::R32G32B32A32_FLOAT:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case TextureFormat::D32_FLOAT_S8_UINT:
    return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

TextureFormat textureFormatFrom(DXGI_FORMAT format) {
  switch (format) {
  case DXGI_FORMAT_R8_UNORM:
    return TextureFormat::R8_UNORM;
  case DXGI_FORMAT_R8G8_UNORM:
    return TextureFormat::R8G8_UNORM;
  case DXGI_FORMAT_R8G8B8A8_UNORM:
    return TextureFormat::R8G8B8A8_UNORM;
  case DXGI_FORMAT_B8G8R8A8_UNORM:
    return TextureFormat::B8G8R8A8_UNORM;
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
    return TextureFormat::R32G32B32A32_FLOAT;
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    return TextureFormat::D32_FLOAT_S8_UINT;
  default:
    return TextureFormat::Invalid;
  }
}

D3D12_FILTER filterFrom(const Sampler::CreateInfo &createInfo) {
  const bool minLinear = createInfo.minFilter == Filter::Linear;
  const bool magLinear = createInfo.magFilter == Filter::Linear;
  const bool mipLinear = createInfo.mipmapMode == MipmapMode::Linear;
  if (minLinear && magLinear && mipLinear)
    return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
  if (minLinear && magLinear)
    return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  if (minLinear)
    return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
  if (magLinear)
    return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
  return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

D3D12_TEXTURE_ADDRESS_MODE addressModeFrom(AddressMode mode) {
  switch (mode) {
  case AddressMode::Repeat:
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  case AddressMode::MirroredRepeat:
    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
  case AddressMode::ClampToEdge:
    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  }
  return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_PRIMITIVE_TOPOLOGY primitiveTopologyFrom(PrimitiveType type) {
  switch (type) {
  case PrimitiveType::TriangleList:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  case PrimitiveType::TriangleStrip:
    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  case PrimitiveType::LineList:
    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
  case PrimitiveType::LineStrip:
    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
  case PrimitiveType::PointList:
    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
  }
  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyTypeFrom(PrimitiveType type) {
  switch (type) {
  case PrimitiveType::LineList:
  case PrimitiveType::LineStrip:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  case PrimitiveType::PointList:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  default:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  }
}

D3D12_FILL_MODE fillModeFrom(FillMode mode) {
  return mode == FillMode::Line ? D3D12_FILL_MODE_WIREFRAME
                                : D3D12_FILL_MODE_SOLID;
}

D3D12_CULL_MODE cullModeFrom(CullMode mode) {
  switch (mode) {
  case CullMode::Front:
    return D3D12_CULL_MODE_FRONT;
  case CullMode::Back:
    return D3D12_CULL_MODE_BACK;
  case CullMode::None:
    return D3D12_CULL_MODE_NONE;
  }
  return D3D12_CULL_MODE_NONE;
}

D3D12_COMPARISON_FUNC compareOpFrom(CompareOp op) {
  switch (op) {
  case CompareOp::Never:
    return D3D12_COMPARISON_FUNC_NEVER;
  case CompareOp::Less:
    return D3D12_COMPARISON_FUNC_LESS;
  case CompareOp::Equal:
    return D3D12_COMPARISON_FUNC_EQUAL;
  case CompareOp::LessOrEqual:
    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
  case CompareOp::Greater:
    return D3D12_COMPARISON_FUNC_GREATER;
  case CompareOp::NotEqual:
    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
  case CompareOp::GreaterOrEqual:
    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
  case CompareOp::Always:
    return D3D12_COMPARISON_FUNC_ALWAYS;
  default:
    return D3D12_COMPARISON_FUNC_ALWAYS;
  }
}

D3D12_STENCIL_OP stencilOpFrom(StencilOp op) {
  switch (op) {
  case StencilOp::Keep:
    return D3D12_STENCIL_OP_KEEP;
  case StencilOp::Zero:
    return D3D12_STENCIL_OP_ZERO;
  case StencilOp::Replace:
    return D3D12_STENCIL_OP_REPLACE;
  case StencilOp::IncrementAndClamp:
    return D3D12_STENCIL_OP_INCR_SAT;
  case StencilOp::DecrementAndClamp:
    return D3D12_STENCIL_OP_DECR_SAT;
  case StencilOp::Invert:
    return D3D12_STENCIL_OP_INVERT;
  case StencilOp::IncrementAndWrap:
    return D3D12_STENCIL_OP_INCR;
  case StencilOp::DecrementAndWrap:
    return D3D12_STENCIL_OP_DECR;
  default:
    return D3D12_STENCIL_OP_KEEP;
  }
}

D3D12_BLEND blendFactorFrom(BlendFactor factor) {
  switch (factor) {
  case BlendFactor::Zero:
    return D3D12_BLEND_ZERO;
  case BlendFactor::One:
    return D3D12_BLEND_ONE;
  case BlendFactor::SrcColor:
    return D3D12_BLEND_SRC_COLOR;
  case BlendFactor::OneMinusSrcColor:
    return D3D12_BLEND_INV_SRC_COLOR;
  case BlendFactor::DstColor:
    return D3D12_BLEND_DEST_COLOR;
  case BlendFactor::OneMinusDstColor:
    return D3D12_BLEND_INV_DEST_COLOR;
  case BlendFactor::SrcAlpha:
    return D3D12_BLEND_SRC_ALPHA;
  case BlendFactor::OneMinusSrcAlpha:
    return D3D12_BLEND_INV_SRC_ALPHA;
  case BlendFactor::DstAlpha:
    return D3D12_BLEND_DEST_ALPHA;
  case BlendFactor::OneMinusDstAlpha:
    return D3D12_BLEND_INV_DEST_ALPHA;
  case BlendFactor::SrcAlphaSaturate:
    return D3D12_BLEND_SRC_ALPHA_SAT;
  default:
    return D3D12_BLEND_ONE;
  }
}

D3D12_BLEND_OP blendOpFrom(BlendOp op) {
  switch (op) {
  case BlendOp::Subtract:
    return D3D12_BLEND_OP_SUBTRACT;
  case BlendOp::ReverseSubtract:
    return D3D12_BLEND_OP_REV_SUBTRACT;
  case BlendOp::Min:
    return D3D12_BLEND_OP_MIN;
  case BlendOp::Max:
    return D3D12_BLEND_OP_MAX;
  default:
    return D3D12_BLEND_OP_ADD;
  }
}

UINT8 colorWriteMaskFrom(UInt8 mask) {
  UINT8 out = 0;
  if (mask & ColorComponent::R)
    out |= D3D12_COLOR_WRITE_ENABLE_RED;
  if (mask & ColorComponent::G)
    out |= D3D12_COLOR_WRITE_ENABLE_GREEN;
  if (mask & ColorComponent::B)
    out |= D3D12_COLOR_WRITE_ENABLE_BLUE;
  if (mask & ColorComponent::A)
    out |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
  return out;
}

DXGI_FORMAT vertexFormatFrom(VertexElementFormat format) {
  switch (format) {
  case VertexElementFormat::Float1:
    return DXGI_FORMAT_R32_FLOAT;
  case VertexElementFormat::Float2:
    return DXGI_FORMAT_R32G32_FLOAT;
  case VertexElementFormat::Float3:
    return DXGI_FORMAT_R32G32B32_FLOAT;
  case VertexElementFormat::Float4:
    return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case VertexElementFormat::UByte4_NORM:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  }
  return DXGI_FORMAT_UNKNOWN;
}

UINT vertexFormatSize(VertexElementFormat format) {
  switch (format) {
  case VertexElementFormat::Float1:
    return sizeof(float);
  case VertexElementFormat::Float2:
    return sizeof(float) * 2;
  case VertexElementFormat::Float3:
    return sizeof(float) * 3;
  case VertexElementFormat::Float4:
    return sizeof(float) * 4;
  case VertexElementFormat::UByte4_NORM:
    return sizeof(uint32_t);
  }
  return 0;
}

DXGI_FORMAT indexFormatFrom(IndexElementSize size) {
  return size == IndexElementSize::Uint16 ? DXGI_FORMAT_R16_UINT
                                          : DXGI_FORMAT_R32_UINT;
}

UINT sampleCountFrom(SampleCount count) {
  switch (count) {
  case SampleCount::x2:
    return 2;
  case SampleCount::x4:
    return 4;
  case SampleCount::x8:
    return 8;
  default:
    return 1;
  }
}
} // namespace sinen::gpu::d3d12::convert

#endif // SINEN_PLATFORM_WINDOWS
