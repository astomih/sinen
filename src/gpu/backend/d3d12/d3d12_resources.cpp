#include "d3d12_resources.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include <cstring>

namespace sinen::gpu::d3d12 {
Buffer::Buffer(const CreateInfo &createInfo, Ptr<Device> device,
               ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state)
    : gpu::Buffer(createInfo), device(device), resource(resource),
      state(state) {}

Texture::Texture(const CreateInfo &createInfo, Ptr<Device> device,
                 ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state,
                 bool swapchainTexture)
    : gpu::Texture(createInfo), device(device), resource(resource),
      state(state), swapchainTexture(swapchainTexture) {}

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

GraphicsPipeline::GraphicsPipeline(const CreateInfo &createInfo,
                                   Ptr<Device> device,
                                   ComPtr<ID3D12RootSignature> rootSignature,
                                   ComPtr<ID3D12PipelineState> pipelineState,
                                   D3D12_PRIMITIVE_TOPOLOGY topology)
    : gpu::GraphicsPipeline(createInfo), device(device),
      rootSignature(rootSignature), pipelineState(pipelineState),
      topology(topology) {}

AccelerationStructure::AccelerationStructure(const CreateInfo &createInfo,
                                             Ptr<Device> device,
                                             ComPtr<ID3D12Resource> resource)
    : gpu::AccelerationStructure(createInfo), device(device),
      resource(resource) {}

RayTracingPipeline::RayTracingPipeline(
    const CreateInfo &createInfo, Ptr<Device> device,
    ComPtr<ID3D12RootSignature> globalRootSignature,
    ComPtr<ID3D12StateObject> stateObject,
    std::vector<std::wstring> shaderGroupExportNames)
    : gpu::RayTracingPipeline(createInfo), device(device),
      globalRootSignature(globalRootSignature), stateObject(stateObject),
      shaderGroupExportNames(std::move(shaderGroupExportNames)) {
  if (stateObject) {
    stateObject.As(&stateObjectProperties);
  }
}

bool RayTracingPipeline::getShaderGroupHandles(UInt32 firstGroup,
                                               UInt32 groupCount, void *dst,
                                               Size dstSize) const {
  const Size handleSize = getShaderGroupHandleSize();
  if (!stateObjectProperties || !dst || dstSize < handleSize * groupCount ||
      firstGroup + groupCount > shaderGroupExportNames.size()) {
    return false;
  }

  auto *out = static_cast<uint8_t *>(dst);
  for (UInt32 i = 0; i < groupCount; ++i) {
    const auto &name = shaderGroupExportNames[firstGroup + i];
    const void *identifier =
        stateObjectProperties->GetShaderIdentifier(name.c_str());
    if (!identifier) {
      return false;
    }
    std::memcpy(out + handleSize * i, identifier, handleSize);
  }
  return true;
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
