#ifndef SINEN_D3D12_RESOURCES_HPP
#define SINEN_D3D12_RESOURCES_HPP

#include <core/def/macro.hpp>

#ifdef SINEN_PLATFORM_WINDOWS

#include <gpu/gpu_buffer.hpp>
#include <gpu/gpu_compute_pipeline.hpp>
#include <gpu/gpu_graphics_pipeline.hpp>
#include <gpu/gpu_sampler.hpp>
#include <gpu/gpu_shader.hpp>
#include <gpu/gpu_texture.hpp>
#include <gpu/gpu_transfer_buffer.hpp>

#include <d3d12.h>
#include <wrl/client.h>

namespace sinen::gpu::d3d12 {
class Device;

using Microsoft::WRL::ComPtr;

struct CpuGpuDescriptor {
  D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
  D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
};

class Buffer : public gpu::Buffer {
public:
  Buffer(const CreateInfo &createInfo, Ptr<Device> device,
         ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state);
  ID3D12Resource *getNative() const { return resource.Get(); }
  D3D12_RESOURCE_STATES getState() const { return state; }
  void setState(D3D12_RESOURCE_STATES value) { state = value; }

private:
  Ptr<Device> device;
  ComPtr<ID3D12Resource> resource;
  D3D12_RESOURCE_STATES state;
};

class Texture : public gpu::Texture {
public:
  Texture(const CreateInfo &createInfo, Ptr<Device> device,
          ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES state,
          bool swapchainTexture = false);
  ID3D12Resource *getNative() const { return resource.Get(); }
  D3D12_RESOURCE_STATES getState() const { return state; }
  void setState(D3D12_RESOURCE_STATES value) { state = value; }
  bool isSwapchainTexture() const { return swapchainTexture; }

  D3D12_CPU_DESCRIPTOR_HANDLE getRtv() const { return rtv; }
  D3D12_CPU_DESCRIPTOR_HANDLE getDsv() const { return dsv; }
  D3D12_CPU_DESCRIPTOR_HANDLE getSrvCpu() const { return srv.cpu; }
  D3D12_GPU_DESCRIPTOR_HANDLE getSrvGpu() const { return srv.gpu; }
  void setRtv(D3D12_CPU_DESCRIPTOR_HANDLE value) { rtv = value; }
  void setDsv(D3D12_CPU_DESCRIPTOR_HANDLE value) { dsv = value; }
  void setSrv(CpuGpuDescriptor value) { srv = value; }

private:
  Ptr<Device> device;
  ComPtr<ID3D12Resource> resource;
  D3D12_RESOURCE_STATES state;
  bool swapchainTexture;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
  D3D12_CPU_DESCRIPTOR_HANDLE dsv{};
  CpuGpuDescriptor srv{};
};

class TransferBuffer : public gpu::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, Ptr<Device> device,
                 ComPtr<ID3D12Resource> resource);
  ~TransferBuffer() override;
  ID3D12Resource *getNative() const { return resource.Get(); }
  void *map(bool cycle) override;
  void unmap() override;

private:
  Ptr<Device> device;
  ComPtr<ID3D12Resource> resource;
  void *mapped = nullptr;
};

class Sampler : public gpu::Sampler {
public:
  Sampler(const CreateInfo &createInfo, Ptr<Device> device,
          CpuGpuDescriptor descriptor);
  D3D12_CPU_DESCRIPTOR_HANDLE getCpu() const { return descriptor.cpu; }
  D3D12_GPU_DESCRIPTOR_HANDLE getGpu() const { return descriptor.gpu; }

private:
  Ptr<Device> device;
  CpuGpuDescriptor descriptor;
};

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, Array<uint8_t> bytecode);
  D3D12_SHADER_BYTECODE getBytecode() const;

private:
  Array<uint8_t> bytecode;
};

class GraphicsPipeline : public gpu::GraphicsPipeline {
public:
  GraphicsPipeline(const CreateInfo &createInfo, Ptr<Device> device,
                   ComPtr<ID3D12RootSignature> rootSignature,
                   ComPtr<ID3D12PipelineState> pipelineState,
                   D3D12_PRIMITIVE_TOPOLOGY topology);
  ID3D12RootSignature *getRootSignature() const { return rootSignature.Get(); }
  ID3D12PipelineState *getNative() const { return pipelineState.Get(); }
  D3D12_PRIMITIVE_TOPOLOGY getTopology() const { return topology; }

private:
  Ptr<Device> device;
  ComPtr<ID3D12RootSignature> rootSignature;
  ComPtr<ID3D12PipelineState> pipelineState;
  D3D12_PRIMITIVE_TOPOLOGY topology;
};

class ComputePipeline : public gpu::ComputePipeline {
public:
  ComputePipeline(const CreateInfo &createInfo, Ptr<Device> device,
                  ComPtr<ID3D12RootSignature> rootSignature,
                  ComPtr<ID3D12PipelineState> pipelineState)
      : gpu::ComputePipeline(createInfo), device(device),
        rootSignature(rootSignature), pipelineState(pipelineState) {}
  ID3D12RootSignature *getRootSignature() const { return rootSignature.Get(); }
  ID3D12PipelineState *getNative() const { return pipelineState.Get(); }

private:
  Ptr<Device> device;
  ComPtr<ID3D12RootSignature> rootSignature;
  ComPtr<ID3D12PipelineState> pipelineState;
};
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS

#endif // SINEN_D3D12_RESOURCES_HPP
