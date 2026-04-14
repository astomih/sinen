#include "d3d12_resources.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include <cstring>

namespace sinen::gpu::d3d12 {
Buffer::Buffer(const CreateInfo &createInfo, Ptr<Device> device,
               ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state)
    : gpu::Buffer(createInfo), device(device), resource(resource), state(state) {
}

Texture::Texture(const CreateInfo &createInfo, Ptr<Device> device,
                 ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state,
                 bool swapchainTexture)
    : gpu::Texture(createInfo), device(device), resource(resource), state(state),
      swapchainTexture(swapchainTexture) {}

TransferBuffer::TransferBuffer(const CreateInfo &createInfo, Ptr<Device> device,
                               ComPtr<ID3D12Resource> resource)
    : gpu::TransferBuffer(createInfo), device(device), resource(resource) {}

TransferBuffer::~TransferBuffer() {
  if (mapped) {
    resource->Unmap(0, nullptr);
    mapped = nullptr;
  }
}

void *TransferBuffer::map(bool) {
  if (mapped) {
    return mapped;
  }
  D3D12_RANGE range{};
  if (getCreateInfo().usage == TransferBufferUsage::Download) {
    range.End = getCreateInfo().size;
  }
  if (FAILED(resource->Map(0, &range, &mapped))) {
    mapped = nullptr;
  }
  return mapped;
}

void TransferBuffer::unmap() {
  if (!mapped) {
    return;
  }
  D3D12_RANGE written{};
  if (getCreateInfo().usage == TransferBufferUsage::Upload) {
    written.End = getCreateInfo().size;
  }
  resource->Unmap(0, &written);
  mapped = nullptr;
}

Sampler::Sampler(const CreateInfo &createInfo, Ptr<Device> device,
                 CpuGpuDescriptor descriptor)
    : gpu::Sampler(createInfo), device(device), descriptor(descriptor) {}

Shader::Shader(const CreateInfo &createInfo, Array<uint8_t> bytecode)
    : gpu::Shader(createInfo), bytecode(std::move(bytecode)) {}

D3D12_SHADER_BYTECODE Shader::getBytecode() const {
  D3D12_SHADER_BYTECODE out{};
  out.pShaderBytecode = bytecode.data();
  out.BytecodeLength = bytecode.size();
  return out;
}

GraphicsPipeline::GraphicsPipeline(
    const CreateInfo &createInfo, Ptr<Device> device,
    ComPtr<ID3D12RootSignature> rootSignature,
    ComPtr<ID3D12PipelineState> pipelineState,
    D3D12_PRIMITIVE_TOPOLOGY topology)
    : gpu::GraphicsPipeline(createInfo), device(device),
      rootSignature(rootSignature), pipelineState(pipelineState),
      topology(topology) {}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
