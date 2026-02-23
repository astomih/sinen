#include "webgpu_convert.hpp"

namespace sinen::gpu::webgpu::convert {
WGPULoadOp LoadOpFrom(LoadOp loadOp) {
  switch (loadOp) {
  case LoadOp::Load:
    return WGPULoadOp_Load;
  case LoadOp::Clear:
    return WGPULoadOp_Clear;
  case LoadOp::DontCare:
    return WGPULoadOp_Clear;
  default:
    return WGPULoadOp_Load;
  }
}

WGPUStoreOp StoreOpFrom(StoreOp storeOp) {
  switch (storeOp) {
  case StoreOp::Store:
    return WGPUStoreOp_Store;
  case StoreOp::DontCare:
    return WGPUStoreOp_Discard;
  default:
    return WGPUStoreOp_Store;
  }
}

WGPUPrimitiveTopology PrimitiveTypeFrom(PrimitiveType primitiveType) {
  switch (primitiveType) {
  case PrimitiveType::TriangleList:
    return WGPUPrimitiveTopology_TriangleList;
  case PrimitiveType::TriangleStrip:
    return WGPUPrimitiveTopology_TriangleStrip;
  case PrimitiveType::LineList:
    return WGPUPrimitiveTopology_LineList;
  case PrimitiveType::LineStrip:
    return WGPUPrimitiveTopology_LineStrip;
  case PrimitiveType::PointList:
    return WGPUPrimitiveTopology_PointList;
  default:
    return WGPUPrimitiveTopology_TriangleList;
  }
}

WGPUTextureFormat TextureFormatFrom(TextureFormat textureFormat) {
  switch (textureFormat) {
  case TextureFormat::Invalid:
    return WGPUTextureFormat_Undefined;
  case TextureFormat::R8_UNORM:
    return WGPUTextureFormat_R8Unorm;
  case TextureFormat::R8G8_UNORM:
    return WGPUTextureFormat_RG8Unorm;
  case TextureFormat::R8G8B8A8_UNORM:
    return WGPUTextureFormat_RGBA8Unorm;
  case TextureFormat::B8G8R8A8_UNORM:
    return WGPUTextureFormat_BGRA8Unorm;
  case TextureFormat::R32G32B32A32_FLOAT:
    return WGPUTextureFormat_RGBA32Float;
  case TextureFormat::D32_FLOAT_S8_UINT:
    return WGPUTextureFormat_Depth32FloatStencil8;
  default:
    return WGPUTextureFormat_Undefined;
  }
}

TextureFormat TextureFormatTo(WGPUTextureFormat textureFormat) {
  switch (textureFormat) {
  case WGPUTextureFormat_R8Unorm:
    return TextureFormat::R8_UNORM;
  case WGPUTextureFormat_RG8Unorm:
    return TextureFormat::R8G8_UNORM;
  case WGPUTextureFormat_RGBA8Unorm:
    return TextureFormat::R8G8B8A8_UNORM;
  case WGPUTextureFormat_BGRA8Unorm:
    return TextureFormat::B8G8R8A8_UNORM;
  case WGPUTextureFormat_RGBA32Float:
    return TextureFormat::R32G32B32A32_FLOAT;
  case WGPUTextureFormat_Depth32FloatStencil8:
    return TextureFormat::D32_FLOAT_S8_UINT;
  default:
    return TextureFormat::Invalid;
  }
}

WGPUTextureDimension TextureDimensionFrom(TextureType textureType) {
  switch (textureType) {
  case TextureType::Texture3D:
    return WGPUTextureDimension_3D;
  case TextureType::Texture2D:
  case TextureType::Texture2DArray:
  case TextureType::Cube:
  case TextureType::CubeArray:
    return WGPUTextureDimension_2D;
  default:
    return WGPUTextureDimension_2D;
  }
}

WGPUTextureViewDimension TextureViewDimensionFrom(TextureType textureType) {
  switch (textureType) {
  case TextureType::Texture2D:
    return WGPUTextureViewDimension_2D;
  case TextureType::Texture2DArray:
    return WGPUTextureViewDimension_2DArray;
  case TextureType::Texture3D:
    return WGPUTextureViewDimension_3D;
  case TextureType::Cube:
    return WGPUTextureViewDimension_Cube;
  case TextureType::CubeArray:
    return WGPUTextureViewDimension_CubeArray;
  default:
    return WGPUTextureViewDimension_2D;
  }
}

WGPUTextureUsage TextureUsageFrom(TextureUsage textureUsage) {
  switch (textureUsage) {
  case TextureUsage::Sampler:
    return WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst |
           WGPUTextureUsage_CopySrc;
  case TextureUsage::ColorTarget:
    return WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding |
           WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
  case TextureUsage::DepthStencilTarget:
    return WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopyDst |
           WGPUTextureUsage_CopySrc;
  default:
    return WGPUTextureUsage_TextureBinding;
  }
}

uint32_t SampleCountFrom(SampleCount sampleCount) {
  switch (sampleCount) {
  case SampleCount::x1:
    return 1;
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

WGPUFilterMode FilterFrom(Filter filter) {
  switch (filter) {
  case Filter::Nearest:
    return WGPUFilterMode_Nearest;
  case Filter::Linear:
    return WGPUFilterMode_Linear;
  default:
    return WGPUFilterMode_Nearest;
  }
}

WGPUMipmapFilterMode MipmapModeFrom(MipmapMode mipmapMode) {
  switch (mipmapMode) {
  case MipmapMode::Nearest:
    return WGPUMipmapFilterMode_Nearest;
  case MipmapMode::Linear:
    return WGPUMipmapFilterMode_Linear;
  default:
    return WGPUMipmapFilterMode_Nearest;
  }
}

WGPUAddressMode AddressModeFrom(AddressMode addressMode) {
  switch (addressMode) {
  case AddressMode::Repeat:
    return WGPUAddressMode_Repeat;
  case AddressMode::MirroredRepeat:
    return WGPUAddressMode_MirrorRepeat;
  case AddressMode::ClampToEdge:
    return WGPUAddressMode_ClampToEdge;
  default:
    return WGPUAddressMode_Repeat;
  }
}

WGPUCompareFunction CompareOpFrom(CompareOp compareOp) {
  switch (compareOp) {
  case CompareOp::Never:
    return WGPUCompareFunction_Never;
  case CompareOp::Less:
    return WGPUCompareFunction_Less;
  case CompareOp::Equal:
    return WGPUCompareFunction_Equal;
  case CompareOp::LessOrEqual:
    return WGPUCompareFunction_LessEqual;
  case CompareOp::Greater:
    return WGPUCompareFunction_Greater;
  case CompareOp::NotEqual:
    return WGPUCompareFunction_NotEqual;
  case CompareOp::GreaterOrEqual:
    return WGPUCompareFunction_GreaterEqual;
  case CompareOp::Always:
    return WGPUCompareFunction_Always;
  case CompareOp::Invalid:
  default:
    return WGPUCompareFunction_Always;
  }
}

WGPUStencilOperation StencilOpFrom(StencilOp op) {
  switch (op) {
  case StencilOp::Keep:
    return WGPUStencilOperation_Keep;
  case StencilOp::Zero:
    return WGPUStencilOperation_Zero;
  case StencilOp::Replace:
    return WGPUStencilOperation_Replace;
  case StencilOp::IncrementAndClamp:
    return WGPUStencilOperation_IncrementClamp;
  case StencilOp::DecrementAndClamp:
    return WGPUStencilOperation_DecrementClamp;
  case StencilOp::Invert:
    return WGPUStencilOperation_Invert;
  case StencilOp::IncrementAndWrap:
    return WGPUStencilOperation_IncrementWrap;
  case StencilOp::DecrementAndWrap:
    return WGPUStencilOperation_DecrementWrap;
  case StencilOp::Invalid:
  default:
    return WGPUStencilOperation_Keep;
  }
}

WGPUBufferUsage BufferUsageFrom(BufferUsage bufferUsage) {
  switch (bufferUsage) {
  case BufferUsage::Vertex:
    return WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst |
           WGPUBufferUsage_CopySrc;
  case BufferUsage::Index:
    return WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst |
           WGPUBufferUsage_CopySrc;
  case BufferUsage::Indirect:
    return WGPUBufferUsage_Indirect | WGPUBufferUsage_CopyDst |
           WGPUBufferUsage_CopySrc;
  default:
    return WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
  }
}

WGPUVertexFormat VertexElementFormatFrom(VertexElementFormat format) {
  switch (format) {
  case VertexElementFormat::Float1:
    return WGPUVertexFormat_Float32;
  case VertexElementFormat::Float2:
    return WGPUVertexFormat_Float32x2;
  case VertexElementFormat::Float3:
    return WGPUVertexFormat_Float32x3;
  case VertexElementFormat::Float4:
    return WGPUVertexFormat_Float32x4;
  case VertexElementFormat::UByte4_NORM:
    return WGPUVertexFormat_Unorm8x4;
  default:
    return WGPUVertexFormat_Float32;
  }
}

WGPUVertexStepMode VertexInputRateFrom(VertexInputRate vertexInputRate) {
  switch (vertexInputRate) {
  case VertexInputRate::Vertex:
    return WGPUVertexStepMode_Vertex;
  case VertexInputRate::Instance:
    return WGPUVertexStepMode_Instance;
  default:
    return WGPUVertexStepMode_Vertex;
  }
}

WGPUCullMode CullModeFrom(CullMode cullMode) {
  switch (cullMode) {
  case CullMode::None:
    return WGPUCullMode_None;
  case CullMode::Front:
    return WGPUCullMode_Front;
  case CullMode::Back:
    return WGPUCullMode_Back;
  default:
    return WGPUCullMode_None;
  }
}

WGPUFrontFace FrontFaceFrom(FrontFace frontFace) {
  switch (frontFace) {
  case FrontFace::Clockwise:
    return WGPUFrontFace_CW;
  case FrontFace::CounterClockwise:
    return WGPUFrontFace_CCW;
  default:
    return WGPUFrontFace_CW;
  }
}

WGPUBlendFactor BlendFactorFrom(BlendFactor blendFactor) {
  switch (blendFactor) {
  case BlendFactor::Zero:
    return WGPUBlendFactor_Zero;
  case BlendFactor::One:
    return WGPUBlendFactor_One;
  case BlendFactor::SrcColor:
    return WGPUBlendFactor_Src;
  case BlendFactor::OneMinusSrcColor:
    return WGPUBlendFactor_OneMinusSrc;
  case BlendFactor::DstColor:
    return WGPUBlendFactor_Dst;
  case BlendFactor::OneMinusDstColor:
    return WGPUBlendFactor_OneMinusDst;
  case BlendFactor::SrcAlpha:
    return WGPUBlendFactor_SrcAlpha;
  case BlendFactor::OneMinusSrcAlpha:
    return WGPUBlendFactor_OneMinusSrcAlpha;
  case BlendFactor::DstAlpha:
    return WGPUBlendFactor_DstAlpha;
  case BlendFactor::OneMinusDstAlpha:
    return WGPUBlendFactor_OneMinusDstAlpha;
  case BlendFactor::ConstantColor:
    return WGPUBlendFactor_Constant;
  case BlendFactor::OneMinusConstantColor:
    return WGPUBlendFactor_OneMinusConstant;
  case BlendFactor::SrcAlphaSaturate:
    return WGPUBlendFactor_SrcAlphaSaturated;
  default:
    return WGPUBlendFactor_Zero;
  }
}

WGPUBlendOperation BlendOpFrom(BlendOp blendOp) {
  switch (blendOp) {
  case BlendOp::Add:
    return WGPUBlendOperation_Add;
  case BlendOp::Subtract:
    return WGPUBlendOperation_Subtract;
  case BlendOp::ReverseSubtract:
    return WGPUBlendOperation_ReverseSubtract;
  case BlendOp::Min:
    return WGPUBlendOperation_Min;
  case BlendOp::Max:
    return WGPUBlendOperation_Max;
  default:
    return WGPUBlendOperation_Add;
  }
}

WGPUColorWriteMask ColorWriteMaskFrom(UInt8 mask) {
  WGPUColorWriteMask out = WGPUColorWriteMask_None;
  if ((mask & gpu::ColorComponent::R) != 0)
    out |= WGPUColorWriteMask_Red;
  if ((mask & gpu::ColorComponent::G) != 0)
    out |= WGPUColorWriteMask_Green;
  if ((mask & gpu::ColorComponent::B) != 0)
    out |= WGPUColorWriteMask_Blue;
  if ((mask & gpu::ColorComponent::A) != 0)
    out |= WGPUColorWriteMask_Alpha;
  return out;
}

WGPUIndexFormat IndexFormatFrom(IndexElementSize size) {
  switch (size) {
  case IndexElementSize::Uint16:
    return WGPUIndexFormat_Uint16;
  case IndexElementSize::Uint32:
    return WGPUIndexFormat_Uint32;
  default:
    return WGPUIndexFormat_Undefined;
  }
}

WGPUOptionalBool OptionalBoolFrom(bool value) {
  return value ? WGPUOptionalBool_True : WGPUOptionalBool_False;
}

WGPUMapMode MapModeFrom(TransferBufferUsage usage) {
  switch (usage) {
  case TransferBufferUsage::Upload:
    return WGPUMapMode_Write;
  case TransferBufferUsage::Download:
    return WGPUMapMode_Read;
  default:
    return WGPUMapMode_None;
  }
}

UInt32 bytesPerPixel(TextureFormat textureFormat) {
  switch (textureFormat) {
  case TextureFormat::R8_UNORM:
    return 1;
  case TextureFormat::R8G8_UNORM:
    return 2;
  case TextureFormat::R8G8B8A8_UNORM:
  case TextureFormat::B8G8R8A8_UNORM:
    return 4;
  case TextureFormat::R32G32B32A32_FLOAT:
    return 16;
  case TextureFormat::D32_FLOAT_S8_UINT:
    return 8;
  case TextureFormat::Invalid:
  default:
    return 4;
  }
}
} // namespace sinen::gpu::webgpu::convert
