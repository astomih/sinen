#include "vulkan_convert.hpp"

#ifndef EMSCRIPTEN
namespace sinen::gpu::vulkan::convert {
VkFormat textureFormatFrom(gpu::TextureFormat format) {
  switch (format) {
  case gpu::TextureFormat::R8_UNORM:
    return VK_FORMAT_R8_UNORM;
  case gpu::TextureFormat::R8G8_UNORM:
    return VK_FORMAT_R8G8_UNORM;
  case gpu::TextureFormat::R8G8B8A8_UNORM:
    return VK_FORMAT_R8G8B8A8_UNORM;
  case gpu::TextureFormat::B8G8R8A8_UNORM:
    return VK_FORMAT_B8G8R8A8_UNORM;
  case gpu::TextureFormat::R32G32B32A32_FLOAT:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case gpu::TextureFormat::D32_FLOAT_S8_UINT:
    return VK_FORMAT_D32_SFLOAT_S8_UINT;
  default:
    return VK_FORMAT_UNDEFINED;
  }
}

VkImageAspectFlags aspectMaskFrom(VkFormat format) {
  switch (format) {
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D16_UNORM:
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  default:
    return VK_IMAGE_ASPECT_COLOR_BIT;
  }
}

VkSampleCountFlagBits sampleCountFrom(gpu::SampleCount sampleCount) {
  switch (sampleCount) {
  case gpu::SampleCount::x1:
    return VK_SAMPLE_COUNT_1_BIT;
  case gpu::SampleCount::x2:
    return VK_SAMPLE_COUNT_2_BIT;
  case gpu::SampleCount::x4:
    return VK_SAMPLE_COUNT_4_BIT;
  case gpu::SampleCount::x8:
    return VK_SAMPLE_COUNT_8_BIT;
  default:
    return VK_SAMPLE_COUNT_1_BIT;
  }
}

VkFilter filterFrom(gpu::Filter filter) {
  switch (filter) {
  case gpu::Filter::Nearest:
    return VK_FILTER_NEAREST;
  case gpu::Filter::Linear:
    return VK_FILTER_LINEAR;
  default:
    return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode mipmapModeFrom(gpu::MipmapMode mode) {
  switch (mode) {
  case gpu::MipmapMode::Nearest:
    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
  case gpu::MipmapMode::Linear:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  default:
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}

VkSamplerAddressMode addressModeFrom(gpu::AddressMode mode) {
  switch (mode) {
  case gpu::AddressMode::Repeat:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  case gpu::AddressMode::MirroredRepeat:
    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case gpu::AddressMode::ClampToEdge:
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  default:
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  }
}

VkCompareOp compareOpFrom(gpu::CompareOp op) {
  switch (op) {
  case gpu::CompareOp::Never:
    return VK_COMPARE_OP_NEVER;
  case gpu::CompareOp::Less:
    return VK_COMPARE_OP_LESS;
  case gpu::CompareOp::Equal:
    return VK_COMPARE_OP_EQUAL;
  case gpu::CompareOp::LessOrEqual:
    return VK_COMPARE_OP_LESS_OR_EQUAL;
  case gpu::CompareOp::Greater:
    return VK_COMPARE_OP_GREATER;
  case gpu::CompareOp::NotEqual:
    return VK_COMPARE_OP_NOT_EQUAL;
  case gpu::CompareOp::GreaterOrEqual:
    return VK_COMPARE_OP_GREATER_OR_EQUAL;
  case gpu::CompareOp::Always:
    return VK_COMPARE_OP_ALWAYS;
  default:
    return VK_COMPARE_OP_ALWAYS;
  }
}

VkPrimitiveTopology primitiveTopologyFrom(gpu::PrimitiveType primitiveType) {
  switch (primitiveType) {
  case gpu::PrimitiveType::TriangleList:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  case gpu::PrimitiveType::TriangleStrip:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  case gpu::PrimitiveType::LineList:
    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  case gpu::PrimitiveType::LineStrip:
    return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
  case gpu::PrimitiveType::PointList:
    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  default:
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }
}

VkPolygonMode polygonModeFrom(gpu::FillMode fillMode) {
  switch (fillMode) {
  case gpu::FillMode::Fill:
    return VK_POLYGON_MODE_FILL;
  case gpu::FillMode::Line:
    return VK_POLYGON_MODE_LINE;
  default:
    return VK_POLYGON_MODE_FILL;
  }
}

VkCullModeFlags cullModeFrom(gpu::CullMode cullMode) {
  switch (cullMode) {
  case gpu::CullMode::None:
    return VK_CULL_MODE_NONE;
  case gpu::CullMode::Front:
    return VK_CULL_MODE_FRONT_BIT;
  case gpu::CullMode::Back:
    return VK_CULL_MODE_BACK_BIT;
  default:
    return VK_CULL_MODE_NONE;
  }
}

VkFrontFace frontFaceFrom(gpu::FrontFace frontFace) {
  switch (frontFace) {
  case gpu::FrontFace::Clockwise:
    return VK_FRONT_FACE_CLOCKWISE;
  case gpu::FrontFace::CounterClockwise:
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
  default:
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
  }
}

VkBlendFactor blendFactorFrom(gpu::BlendFactor factor) {
  switch (factor) {
  case gpu::BlendFactor::Zero:
    return VK_BLEND_FACTOR_ZERO;
  case gpu::BlendFactor::One:
    return VK_BLEND_FACTOR_ONE;
  case gpu::BlendFactor::SrcColor:
    return VK_BLEND_FACTOR_SRC_COLOR;
  case gpu::BlendFactor::OneMinusSrcColor:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
  case gpu::BlendFactor::DstColor:
    return VK_BLEND_FACTOR_DST_COLOR;
  case gpu::BlendFactor::OneMinusDstColor:
    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
  case gpu::BlendFactor::SrcAlpha:
    return VK_BLEND_FACTOR_SRC_ALPHA;
  case gpu::BlendFactor::OneMinusSrcAlpha:
    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  case gpu::BlendFactor::DstAlpha:
    return VK_BLEND_FACTOR_DST_ALPHA;
  case gpu::BlendFactor::OneMinusDstAlpha:
    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
  case gpu::BlendFactor::ConstantColor:
    return VK_BLEND_FACTOR_CONSTANT_COLOR;
  case gpu::BlendFactor::OneMinusConstantColor:
    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
  case gpu::BlendFactor::SrcAlphaSaturate:
    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
  default:
    return VK_BLEND_FACTOR_ONE;
  }
}

VkBlendOp blendOpFrom(gpu::BlendOp op) {
  switch (op) {
  case gpu::BlendOp::Add:
    return VK_BLEND_OP_ADD;
  case gpu::BlendOp::Subtract:
    return VK_BLEND_OP_SUBTRACT;
  case gpu::BlendOp::ReverseSubtract:
    return VK_BLEND_OP_REVERSE_SUBTRACT;
  case gpu::BlendOp::Min:
    return VK_BLEND_OP_MIN;
  case gpu::BlendOp::Max:
    return VK_BLEND_OP_MAX;
  default:
    return VK_BLEND_OP_ADD;
  }
}

VkVertexInputRate vertexInputRateFrom(gpu::VertexInputRate rate) {
  switch (rate) {
  case gpu::VertexInputRate::Vertex:
    return VK_VERTEX_INPUT_RATE_VERTEX;
  case gpu::VertexInputRate::Instance:
    return VK_VERTEX_INPUT_RATE_INSTANCE;
  default:
    return VK_VERTEX_INPUT_RATE_VERTEX;
  }
}

VkFormat vertexElementFormatFrom(gpu::VertexElementFormat format) {
  switch (format) {
  case gpu::VertexElementFormat::Float1:
    return VK_FORMAT_R32_SFLOAT;
  case gpu::VertexElementFormat::Float2:
    return VK_FORMAT_R32G32_SFLOAT;
  case gpu::VertexElementFormat::Float3:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case gpu::VertexElementFormat::Float4:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case gpu::VertexElementFormat::UByte4_NORM:
    return VK_FORMAT_R8G8B8A8_UNORM;
  default:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  }
}

VkAttachmentLoadOp loadOpFrom(gpu::LoadOp op) {
  switch (op) {
  case gpu::LoadOp::Load:
    return VK_ATTACHMENT_LOAD_OP_LOAD;
  case gpu::LoadOp::Clear:
    return VK_ATTACHMENT_LOAD_OP_CLEAR;
  case gpu::LoadOp::DontCare:
    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  default:
    return VK_ATTACHMENT_LOAD_OP_LOAD;
  }
}

VkAttachmentStoreOp storeOpFrom(gpu::StoreOp op) {
  switch (op) {
  case gpu::StoreOp::Store:
    return VK_ATTACHMENT_STORE_OP_STORE;
  case gpu::StoreOp::DontCare:
    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
  default:
    return VK_ATTACHMENT_STORE_OP_STORE;
  }
}
} // namespace sinen::gpu::vulkan::convert
#endif // EMSCRIPTEN

