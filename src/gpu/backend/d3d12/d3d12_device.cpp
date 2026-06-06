#include "d3d12_device.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_command.hpp"
#include "d3d12_convert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <core/logger/log.hpp>

#include <algorithm>
#include <cstring>

namespace sinen::gpu::d3d12 {
namespace {
D3D12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE type) {
  D3D12_HEAP_PROPERTIES props{};
  props.Type = type;
  props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  props.CreationNodeMask = 1;
  props.VisibleNodeMask = 1;
  return props;
}

D3D12_RESOURCE_DESC bufferDesc(UINT64 size) {
  D3D12_RESOURCE_DESC desc{};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Alignment = 0;
  desc.Width = size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  return desc;
}

UINT64 alignTo(UINT64 value, UINT64 alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

void logIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s (HRESULT 0x%08X)", message,
                 static_cast<unsigned int>(hr));
  }
}

bool hasRequiredD3D12Options(ID3D12Device *device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
  if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options,
                                         sizeof(options)))) {
    return false;
  }
  return options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_2;
}

bool hasRayTracingSupport(ID3D12Device *device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 options{};
  if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options,
                                         sizeof(options)))) {
    return false;
  }
  return options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

bool hasRayQuerySupport(ID3D12Device *device) {
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 options{};
  if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options,
                                         sizeof(options)))) {
    return false;
  }
  return options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1;
}

UINT16 depthOrArraySizeFrom(const gpu::Texture::CreateInfo &createInfo) {
  switch (createInfo.type) {
  case TextureType::Texture3D:
    return static_cast<UINT16>(createInfo.layerCountOrDepth);
  case TextureType::Cube:
    return 6;
  case TextureType::CubeArray:
    return static_cast<UINT16>(createInfo.layerCountOrDepth * 6);
  default:
    return static_cast<UINT16>(
        std::max<UInt32>(1, createInfo.layerCountOrDepth));
  }
}

struct D3D12VertexSemantic {
  const char *name;
  UINT index;
};

D3D12VertexSemantic
vertexSemanticFrom(const gpu::VertexInputState &vertexInputState,
                   const gpu::VertexAttribute &attr) {
  const auto &vb = vertexInputState.vertexBufferDescriptions[attr.bufferSlot];
  if (vb.inputRate == VertexInputRate::Instance) {
    return {"TEXCOORD", attr.location};
  }

  if (attr.bufferSlot == 0) {
    switch (attr.location) {
    case 0:
      return {"POSITION", 0};
    case 1:
      return {"NORMAL", 0};
    case 2:
      return {"TEXCOORD", 0};
    case 3:
      return {"COLOR", 0};
    default:
      return {"TEXCOORD", attr.location};
    }
  }

  size_t slotAttributeCount = 0;
  size_t slotAttributeIndex = 0;
  for (const auto &other : vertexInputState.vertexAttributes) {
    if (other.bufferSlot != attr.bufferSlot) {
      continue;
    }
    if (other.location < attr.location) {
      ++slotAttributeIndex;
    }
    ++slotAttributeCount;
  }

  if (slotAttributeCount == 1) {
    return {"TANGENT", 0};
  }
  if (slotAttributeIndex == 0) {
    return {"BLENDINDICES", 0};
  }
  if (slotAttributeIndex == 1) {
    return {"BLENDWEIGHT", 0};
  }
  return {"TEXCOORD", attr.location};
}
} // namespace

Device::Device(const CreateInfo &createInfo) : gpu::Device(createInfo) {
  createDeviceObjects();
}

Device::~Device() {
  waitForGpuIdle();
  destroySwapchain();
  if (fenceEvent) {
    CloseHandle(static_cast<HANDLE>(fenceEvent));
    fenceEvent = nullptr;
  }
}

void Device::createDeviceObjects() {
  UINT factoryFlags = 0;
  if (this->getCreateInfo().debugMode) {
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
      debug->EnableDebugLayer();
      factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
  }

  logIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)),
              "D3D12: CreateDXGIFactory2 failed");

  for (UINT i = 0;
       factory && factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
       ++i) {
    DXGI_ADAPTER_DESC1 desc{};
    adapter->GetDesc1(&desc);
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }
    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&device))) &&
        hasRequiredD3D12Options(device.Get())) {
      break;
    }
    adapter.Reset();
    device.Reset();
  }
  if (!device && factory) {
    ComPtr<IDXGIAdapter> warp;
    if (SUCCEEDED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)))) {
      D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0,
                        IID_PPV_ARGS(&device));
      if (device && !hasRequiredD3D12Options(device.Get())) {
        device.Reset();
      }
    }
  }
  if (!device) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: no adapter supports Feature Level 11_0 and Resource "
                 "Binding Tier 2");
    return;
  }
  if (hasRayTracingSupport(device.Get())) {
    HRESULT hr = device.As(&device5);
    if (SUCCEEDED(hr)) {
      rayTracingSupported = true;
      rayQuerySupported = hasRayQuerySupport(device.Get());
      if (!rayQuerySupported) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "D3D12: ray query disabled; DXR tier 1.1 is not "
                    "supported by this device");
      }
    } else {
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                  "D3D12: ray tracing disabled; ID3D12Device5 is not "
                  "available (HRESULT 0x%08X)",
                  static_cast<unsigned int>(hr));
      rayTracingSupported = false;
      rayQuerySupported = false;
    }
  } else {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: ray tracing disabled; DXR is not supported by this "
                "device");
  }

  D3D12_COMMAND_QUEUE_DESC queueDesc{};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  HRESULT hr =
      device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
  logIfFailed(hr, "D3D12: CreateCommandQueue failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
  srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srvDesc.NumDescriptors = SrvHeapCapacity;
  srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  hr = device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&srvHeap));
  logIfFailed(hr, "D3D12: create shader-visible SRV heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }
  srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&srvCpuHeap));
  logIfFailed(hr, "D3D12: create CPU SRV heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  D3D12_DESCRIPTOR_HEAP_DESC samplerDesc{};
  samplerDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  samplerDesc.NumDescriptors = SamplerHeapCapacity;
  samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  hr = device->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(&samplerHeap));
  logIfFailed(hr, "D3D12: create shader-visible sampler heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }
  samplerDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr =
      device->CreateDescriptorHeap(&samplerDesc, IID_PPV_ARGS(&samplerCpuHeap));
  logIfFailed(hr, "D3D12: create CPU sampler heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
  rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvDesc.NumDescriptors = RtvHeapCapacity;
  hr = device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvHeap));
  logIfFailed(hr, "D3D12: create RTV heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  D3D12_DESCRIPTOR_HEAP_DESC dsvDesc{};
  dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvDesc.NumDescriptors = DsvHeapCapacity;
  hr = device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&dsvHeap));
  logIfFailed(hr, "D3D12: create DSV heap failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  srvDescriptorSize = device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  rtvDescriptorSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  dsvDescriptorSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  logIfFailed(hr, "D3D12: CreateFence failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: device initialization failed");
    return;
  }
  fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (!fenceEvent) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "D3D12: CreateEvent failed");
    disableRayTracing("D3D12: device initialization failed");
    return;
  }

  createGraphicsRootSignature();
  createComputeRootSignature();
  createRayTracingRootSignature();
  createDefaultDescriptors();
}

void Device::claimWindow(void *window) {
  if (deviceLost || !isValid()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: cannot claim window because device is unavailable");
    return;
  }
  this->window = static_cast<SDL_Window *>(window);
  SDL_PropertiesID props = SDL_GetWindowProperties(this->window);
  hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                                nullptr);
  if (!hwnd) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "D3D12: missing HWND");
    return;
  }
  createSwapchain();
}

void Device::createSwapchain() {
  if (deviceLost || !isValid()) {
    return;
  }
  int w = 0;
  int h = 0;
  SDL_GetWindowSizeInPixels(window, &w, &h);
  width = max(1, w);
  height = max(1, h);

  if (swapchain) {
    resizeSwapchain(width, height);
    return;
  }

  DXGI_SWAP_CHAIN_DESC1 desc{};
  desc.Width = width;
  desc.Height = height;
  desc.Format = swapchainFormat;
  desc.SampleDesc.Count = 1;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = FrameCount;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

  ComPtr<IDXGISwapChain1> swapchain1;
  HRESULT hr = factory->CreateSwapChainForHwnd(commandQueue.Get(),
                                               static_cast<HWND>(hwnd), &desc,
                                               nullptr, nullptr, &swapchain1);
  logIfFailed(hr, "D3D12: CreateSwapChainForHwnd failed");
  if (FAILED(hr)) {
    return;
  }
  factory->MakeWindowAssociation(static_cast<HWND>(hwnd),
                                 DXGI_MWA_NO_ALT_ENTER);
  swapchain1.As(&swapchain);
  currentBackBuffer = swapchain->GetCurrentBackBufferIndex();
  createSwapchainTextures();
}

void Device::resizeSwapchain(UINT newWidth, UINT newHeight) {
  waitForGpuIdle();
  swapchainTextures.clear();
  HRESULT hr = swapchain->ResizeBuffers(FrameCount, newWidth, newHeight,
                                        swapchainFormat, 0);
  logIfFailed(hr, "D3D12: ResizeBuffers failed");
  if (FAILED(hr)) {
    destroySwapchain();
    return;
  }
  width = newWidth;
  height = newHeight;
  currentBackBuffer = swapchain->GetCurrentBackBufferIndex();
  createSwapchainTextures();
}

void Device::createSwapchainTextures() {
  swapchainTextures.clear();
  if (swapchainRtvs.size() != FrameCount) {
    swapchainRtvs.clear();
    swapchainRtvs.reserve(FrameCount);
    for (UINT i = 0; i < FrameCount; ++i) {
      swapchainRtvs.push_back(allocateRtvDescriptor());
    }
  }
  for (UINT i = 0; i < FrameCount; ++i) {
    ComPtr<ID3D12Resource> backBuffer;
    HRESULT hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
    logIfFailed(hr, "D3D12: GetBuffer failed");
    if (FAILED(hr)) {
      continue;
    }

    gpu::Texture::CreateInfo ci{};
    ci.allocator = getCreateInfo().allocator;
    ci.type = TextureType::Texture2D;
    ci.format = convert::textureFormatFrom(swapchainFormat);
    ci.usage = TextureUsage::ColorTarget;
    ci.width = width;
    ci.height = height;
    ci.layerCountOrDepth = 1;
    ci.numLevels = 1;
    ci.sampleCount = SampleCount::x1;
    auto texture = makePtr<Texture>(ci.allocator, ci, get(), backBuffer,
                                    D3D12_RESOURCE_STATE_PRESENT, true);
    auto rtv = swapchainRtvs[i];
    device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv);
    texture->setRtv(rtv);
    swapchainTextures.push_back(texture);
  }
}

void Device::destroySwapchain() {
  swapchainTextures.clear();
  swapchain.Reset();
}

void Device::createDefaultDescriptors() {
  defaultSrv = allocateSrvDescriptor();
  D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
  srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv.Texture2D.MipLevels = 1;
  device->CreateShaderResourceView(nullptr, &srv, defaultSrv.cpu);

  gpu::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = getCreateInfo().allocator;
  samplerInfo.minFilter = Filter::Linear;
  samplerInfo.magFilter = Filter::Linear;
  samplerInfo.mipmapMode = MipmapMode::Linear;
  samplerInfo.addressModeU = AddressMode::Repeat;
  samplerInfo.addressModeV = AddressMode::Repeat;
  samplerInfo.addressModeW = AddressMode::Repeat;
  defaultSampler = allocateSamplerDescriptor();
  D3D12_SAMPLER_DESC sampler{};
  sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
  sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  sampler.MaxLOD = D3D12_FLOAT32_MAX;
  device->CreateSampler(&sampler, defaultSampler.cpu);
}

void Device::createGraphicsRootSignature() {
  D3D12_DESCRIPTOR_RANGE ranges[5]{};
  ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[0].NumDescriptors = 16;
  ranges[0].BaseShaderRegister = 0;
  ranges[0].RegisterSpace = 2;
  ranges[0].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  ranges[1].NumDescriptors = 16;
  ranges[1].BaseShaderRegister = 0;
  ranges[1].RegisterSpace = 2;
  ranges[1].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[2].NumDescriptors = 16;
  ranges[2].BaseShaderRegister = 0;
  ranges[2].RegisterSpace = 0;
  ranges[2].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  ranges[3].NumDescriptors = 16;
  ranges[3].BaseShaderRegister = 0;
  ranges[3].RegisterSpace = 0;
  ranges[3].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[4].NumDescriptors = 8;
  ranges[4].BaseShaderRegister = 0;
  ranges[4].RegisterSpace = 6;
  ranges[4].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  D3D12_ROOT_PARAMETER params[13]{};
  params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[0].Descriptor.ShaderRegister = 0;
  params[0].Descriptor.RegisterSpace = 1;
  params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[1].Descriptor.ShaderRegister = 1;
  params[1].Descriptor.RegisterSpace = 1;
  params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[2].Descriptor.ShaderRegister = 0;
  params[2].Descriptor.RegisterSpace = 3;
  params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[3].Descriptor.ShaderRegister = 1;
  params[3].Descriptor.RegisterSpace = 3;
  params[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[4].DescriptorTable.NumDescriptorRanges = 1;
  params[4].DescriptorTable.pDescriptorRanges = &ranges[0];
  params[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[5].DescriptorTable.NumDescriptorRanges = 1;
  params[5].DescriptorTable.pDescriptorRanges = &ranges[1];
  params[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[6].Descriptor.ShaderRegister = 0;
  params[6].Descriptor.RegisterSpace = 0;
  params[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  params[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[7].Descriptor.ShaderRegister = 1;
  params[7].Descriptor.RegisterSpace = 0;
  params[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  params[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[8].Descriptor.ShaderRegister = 0;
  params[8].Descriptor.RegisterSpace = 0;
  params[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
  params[9].Descriptor.ShaderRegister = 1;
  params[9].Descriptor.RegisterSpace = 0;
  params[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[10].DescriptorTable.NumDescriptorRanges = 1;
  params[10].DescriptorTable.pDescriptorRanges = &ranges[2];
  params[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[11].DescriptorTable.NumDescriptorRanges = 1;
  params[11].DescriptorTable.pDescriptorRanges = &ranges[3];
  params[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  params[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[12].DescriptorTable.NumDescriptorRanges = 1;
  params[12].DescriptorTable.pDescriptorRanges = &ranges[4];
  params[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

  D3D12_ROOT_SIGNATURE_DESC desc{};
  desc.NumParameters = 13;
  desc.pParameters = params;
  desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  ComPtr<ID3DBlob> blob;
  ComPtr<ID3DBlob> error;
  HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                           &blob, &error);
  if (FAILED(hr)) {
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "D3D12 root signature: %s",
                   static_cast<const char *>(error->GetBufferPointer()));
    }
    return;
  }
  device->CreateRootSignature(0, blob->GetBufferPointer(),
                              blob->GetBufferSize(),
                              IID_PPV_ARGS(&graphicsRootSignature));
}

ID3D12RootSignature *Device::getGraphicsRootSignature() {
  return graphicsRootSignature.Get();
}

void Device::createComputeRootSignature() {
  D3D12_DESCRIPTOR_RANGE ranges[2]{};
  ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  ranges[0].NumDescriptors = 16;
  ranges[0].BaseShaderRegister = 0;
  ranges[0].RegisterSpace = 0;
  ranges[0].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[1].NumDescriptors = 8;
  ranges[1].BaseShaderRegister = 0;
  ranges[1].RegisterSpace = 6;
  ranges[1].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  D3D12_ROOT_PARAMETER params[6]{};
  params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[0].DescriptorTable.NumDescriptorRanges = 1;
  params[0].DescriptorTable.pDescriptorRanges = &ranges[0];
  params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  for (UINT i = 0; i < 4; ++i) {
    params[i + 1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[i + 1].Descriptor.ShaderRegister = i;
    params[i + 1].Descriptor.RegisterSpace = 0;
    params[i + 1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  }
  params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[5].DescriptorTable.NumDescriptorRanges = 1;
  params[5].DescriptorTable.pDescriptorRanges = &ranges[1];
  params[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

  D3D12_ROOT_SIGNATURE_DESC desc{};
  desc.NumParameters = 6;
  desc.pParameters = params;

  ComPtr<ID3DBlob> blob;
  ComPtr<ID3DBlob> error;
  HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                           &blob, &error);
  if (FAILED(hr)) {
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "D3D12 compute root signature: %s",
                   static_cast<const char *>(error->GetBufferPointer()));
    }
    return;
  }
  device->CreateRootSignature(0, blob->GetBufferPointer(),
                              blob->GetBufferSize(),
                              IID_PPV_ARGS(&computeRootSignature));
}

ID3D12RootSignature *Device::getComputeRootSignature() {
  return computeRootSignature.Get();
}

void Device::createRayTracingRootSignature() {
  if (!rayTracingSupported) {
    return;
  }

  D3D12_DESCRIPTOR_RANGE ranges[2]{};
  ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[0].NumDescriptors = 32;
  ranges[0].BaseShaderRegister = 0;
  ranges[0].RegisterSpace = 4;
  ranges[0].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  ranges[1].NumDescriptors = 32;
  ranges[1].BaseShaderRegister = 0;
  ranges[1].RegisterSpace = 4;
  ranges[1].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  D3D12_ROOT_PARAMETER params[6]{};
  params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[0].DescriptorTable.NumDescriptorRanges = 1;
  params[0].DescriptorTable.pDescriptorRanges = &ranges[0];
  params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  params[1].DescriptorTable.NumDescriptorRanges = 1;
  params[1].DescriptorTable.pDescriptorRanges = &ranges[1];
  params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  for (UINT i = 0; i < 4; ++i) {
    params[i + 2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[i + 2].Descriptor.ShaderRegister = i;
    params[i + 2].Descriptor.RegisterSpace = 5;
    params[i + 2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  }

  D3D12_ROOT_SIGNATURE_DESC desc{};
  desc.NumParameters = 6;
  desc.pParameters = params;

  ComPtr<ID3DBlob> blob;
  ComPtr<ID3DBlob> error;
  HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                           &blob, &error);
  if (FAILED(hr)) {
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "D3D12 ray tracing root signature: %s",
                   static_cast<const char *>(error->GetBufferPointer()));
    }
    disableRayTracing("D3D12: ray tracing root signature serialization failed");
    return;
  }
  hr = device->CreateRootSignature(0, blob->GetBufferPointer(),
                                   blob->GetBufferSize(),
                                   IID_PPV_ARGS(&rayTracingRootSignature));
  logIfFailed(hr, "D3D12: create ray tracing root signature failed");
  if (FAILED(hr)) {
    disableRayTracing("D3D12: ray tracing root signature creation failed");
  }
}

ID3D12RootSignature *Device::getRayTracingRootSignature() {
  return rayTracingRootSignature.Get();
}

Ptr<gpu::Buffer>
Device::createBuffer(const gpu::Buffer::CreateInfo &createInfo) {
  ComPtr<ID3D12Resource> resource;
  auto desc = bufferDesc(createInfo.size);
  if (createInfo.usage == BufferUsage::Storage) {
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }
  auto heap = heapProperties(D3D12_HEAP_TYPE_DEFAULT);
  auto initialState = createInfo.usage == BufferUsage::Storage
                          ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS
                          : D3D12_RESOURCE_STATE_COMMON;
  HRESULT hr = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
                                               &desc, initialState, nullptr,
                                               IID_PPV_ARGS(&resource));
  logIfFailed(hr, "D3D12: create buffer failed");
  if (FAILED(hr)) {
    return nullptr;
  }
  return makePtr<Buffer>(createInfo.allocator, createInfo, get(), resource,
                         initialState);
}

Ptr<gpu::TransferBuffer> Device::createTransferBuffer(
    const gpu::TransferBuffer::CreateInfo &createInfo) {
  ComPtr<ID3D12Resource> resource;
  const auto heapType = createInfo.usage == TransferBufferUsage::Upload
                            ? D3D12_HEAP_TYPE_UPLOAD
                            : D3D12_HEAP_TYPE_READBACK;
  const auto initialState = createInfo.usage == TransferBufferUsage::Upload
                                ? D3D12_RESOURCE_STATE_GENERIC_READ
                                : D3D12_RESOURCE_STATE_COPY_DEST;
  auto heap = heapProperties(heapType);
  auto desc = bufferDesc(createInfo.size);
  HRESULT hr = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
                                               &desc, initialState, nullptr,
                                               IID_PPV_ARGS(&resource));
  logIfFailed(hr, "D3D12: create transfer buffer failed");
  if (FAILED(hr)) {
    return nullptr;
  }
  return makePtr<TransferBuffer>(createInfo.allocator, createInfo, get(),
                                 resource);
}

Ptr<gpu::Texture>
Device::createTexture(const gpu::Texture::CreateInfo &createInfo) {
  ComPtr<ID3D12Resource> resource;
  D3D12_RESOURCE_DESC desc{};
  desc.Dimension = convert::textureDimensionFrom(createInfo.type);
  desc.Width = createInfo.width;
  desc.Height = createInfo.height;
  desc.DepthOrArraySize = depthOrArraySizeFrom(createInfo);
  desc.MipLevels = static_cast<UINT16>(createInfo.numLevels);
  desc.Format = convert::textureFormatFrom(createInfo.format);
  desc.SampleDesc.Count = convert::sampleCountFrom(createInfo.sampleCount);
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Flags = convert::textureFlagsFrom(createInfo.usage);

  D3D12_CLEAR_VALUE clear{};
  D3D12_CLEAR_VALUE *clearPtr = nullptr;
  if (createInfo.usage == TextureUsage::ColorTarget) {
    clear.Format = desc.Format;
    clear.Color[3] = 1.0f;
    clearPtr = &clear;
  } else if (createInfo.usage == TextureUsage::DepthStencilTarget) {
    clear.Format = desc.Format;
    clear.DepthStencil.Depth = 1.0f;
    clearPtr = &clear;
  }

  auto initialState = createInfo.usage == TextureUsage::Sampler
                          ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
                      : createInfo.usage == TextureUsage::Storage
                          ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS
                          : D3D12_RESOURCE_STATE_COMMON;
  auto heap = heapProperties(D3D12_HEAP_TYPE_DEFAULT);
  HRESULT hr = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
                                               &desc, initialState, clearPtr,
                                               IID_PPV_ARGS(&resource));
  logIfFailed(hr, "D3D12: create texture failed");
  if (FAILED(hr)) {
    return nullptr;
  }

  auto texture = makePtr<Texture>(createInfo.allocator, createInfo, get(),
                                  resource, initialState, false);
  if (createInfo.usage == TextureUsage::Sampler) {
    auto srv = allocateSrvDescriptor();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if (createInfo.type == TextureType::Texture2DArray) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      srvDesc.Texture2DArray.ArraySize = createInfo.layerCountOrDepth;
      srvDesc.Texture2DArray.MipLevels = createInfo.numLevels;
    } else if (createInfo.type == TextureType::Cube) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srvDesc.TextureCube.MipLevels = createInfo.numLevels;
    } else if (createInfo.type == TextureType::CubeArray) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
      srvDesc.TextureCubeArray.NumCubes = createInfo.layerCountOrDepth;
      srvDesc.TextureCubeArray.MipLevels = createInfo.numLevels;
    } else if (createInfo.type == TextureType::Texture3D) {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      srvDesc.Texture3D.MipLevels = createInfo.numLevels;
    } else {
      srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = createInfo.numLevels;
    }
    device->CreateShaderResourceView(resource.Get(), &srvDesc, srv.cpu);
    texture->setSrv(srv);
  } else if (createInfo.usage == TextureUsage::ColorTarget) {
    auto rtv = allocateRtvDescriptor();
    device->CreateRenderTargetView(resource.Get(), nullptr, rtv);
    texture->setRtv(rtv);
  } else if (createInfo.usage == TextureUsage::DepthStencilTarget) {
    auto dsv = allocateDsvDescriptor();
    device->CreateDepthStencilView(resource.Get(), nullptr, dsv);
    texture->setDsv(dsv);
  }
  return texture;
}

Ptr<gpu::Sampler>
Device::createSampler(const gpu::Sampler::CreateInfo &createInfo) {
  auto descriptor = allocateSamplerDescriptor();
  D3D12_SAMPLER_DESC desc{};
  desc.Filter = convert::filterFrom(createInfo);
  desc.AddressU = convert::addressModeFrom(createInfo.addressModeU);
  desc.AddressV = convert::addressModeFrom(createInfo.addressModeV);
  desc.AddressW = convert::addressModeFrom(createInfo.addressModeW);
  desc.MipLODBias = createInfo.mipLodBias;
  desc.MaxAnisotropy = createInfo.maxAnisotropy > 0
                           ? static_cast<UINT>(createInfo.maxAnisotropy)
                           : 1;
  desc.ComparisonFunc = convert::compareOpFrom(createInfo.compareOp);
  desc.MinLOD = createInfo.minLod;
  desc.MaxLOD =
      createInfo.maxLod > 0.0f ? createInfo.maxLod : D3D12_FLOAT32_MAX;
  device->CreateSampler(&desc, descriptor.cpu);
  return makePtr<Sampler>(createInfo.allocator, createInfo, get(), descriptor);
}

Ptr<gpu::Shader>
Device::createShader(const gpu::Shader::CreateInfo &createInfo) {
  if (createInfo.format != ShaderFormat::DXIL &&
      createInfo.format != ShaderFormat::DXBC) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12 backend requires DXIL or DXBC shaders");
    return nullptr;
  }
  Array<uint8_t> bytecode(createInfo.allocator);
  bytecode.resize(createInfo.size);
  std::memcpy(bytecode.data(), createInfo.data, createInfo.size);
  return makePtr<Shader>(createInfo.allocator, createInfo, std::move(bytecode));
}

Ptr<gpu::GraphicsPipeline> Device::createGraphicsPipeline(
    const gpu::GraphicsPipeline::CreateInfo &createInfo) {
  Array<D3D12_INPUT_ELEMENT_DESC> elements(createInfo.allocator);
  elements.resize(createInfo.vertexInputState.vertexAttributes.size());

  for (size_t i = 0; i < elements.size(); ++i) {
    const auto &attr = createInfo.vertexInputState.vertexAttributes[i];
    const auto &vb =
        createInfo.vertexInputState.vertexBufferDescriptions[attr.bufferSlot];

    const auto semantic = vertexSemanticFrom(createInfo.vertexInputState, attr);
    elements[i].SemanticName = semantic.name;
    elements[i].SemanticIndex = semantic.index;
    elements[i].Format = convert::vertexFormatFrom(attr.format);
    elements[i].InputSlot = attr.bufferSlot;
    elements[i].AlignedByteOffset = attr.offset;
    elements[i].InputSlotClass =
        vb.inputRate == VertexInputRate::Instance
            ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
            : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    elements[i].InstanceDataStepRate =
        vb.inputRate == VertexInputRate::Instance ? 1 : 0;
  }

  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
  desc.pRootSignature = graphicsRootSignature.Get();
  desc.VS = downCast<Shader>(createInfo.vertexShader)->getBytecode();
  desc.PS = downCast<Shader>(createInfo.fragmentShader)->getBytecode();
  desc.BlendState.AlphaToCoverageEnable = FALSE;
  desc.BlendState.IndependentBlendEnable = FALSE;
  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
    auto &rt = desc.BlendState.RenderTarget[i];
    rt.BlendEnable = FALSE;
    rt.LogicOpEnable = FALSE;
    rt.SrcBlend = D3D12_BLEND_ONE;
    rt.DestBlend = D3D12_BLEND_ZERO;
    rt.BlendOp = D3D12_BLEND_OP_ADD;
    rt.SrcBlendAlpha = D3D12_BLEND_ONE;
    rt.DestBlendAlpha = D3D12_BLEND_ZERO;
    rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
  }
  for (size_t i = 0; i < createInfo.targetInfo.colorTargetDescriptions.size();
       ++i) {
    const auto &src = createInfo.targetInfo.colorTargetDescriptions[i];
    auto &dst = desc.BlendState.RenderTarget[i];
    dst.BlendEnable = src.blendState.enableBlend;
    dst.SrcBlend = convert::blendFactorFrom(src.blendState.srcColorBlendFactor);
    dst.DestBlend =
        convert::blendFactorFrom(src.blendState.dstColorBlendFactor);
    dst.BlendOp = convert::blendOpFrom(src.blendState.colorBlendOp);
    dst.SrcBlendAlpha =
        convert::blendFactorFrom(src.blendState.srcAlphaBlendFactor);
    dst.DestBlendAlpha =
        convert::blendFactorFrom(src.blendState.dstAlphaBlendFactor);
    dst.BlendOpAlpha = convert::blendOpFrom(src.blendState.alphaBlendOp);
    dst.RenderTargetWriteMask =
        src.blendState.enableColorWriteMask
            ? convert::colorWriteMaskFrom(src.blendState.colorWriteMask)
            : D3D12_COLOR_WRITE_ENABLE_ALL;
    desc.RTVFormats[i] = convert::textureFormatFrom(src.format);
  }
  desc.SampleMask = UINT_MAX;
  desc.RasterizerState.FillMode =
      convert::fillModeFrom(createInfo.rasterizerState.fillMode);
  desc.RasterizerState.CullMode =
      convert::cullModeFrom(createInfo.rasterizerState.cullMode);
  desc.RasterizerState.FrontCounterClockwise =
      createInfo.rasterizerState.frontFace == FrontFace::CounterClockwise;
  desc.RasterizerState.DepthBias =
      static_cast<INT>(createInfo.rasterizerState.depthBiasConstantFactor);
  desc.RasterizerState.DepthBiasClamp =
      createInfo.rasterizerState.depthBiasClamp;
  desc.RasterizerState.SlopeScaledDepthBias =
      createInfo.rasterizerState.depthBiasSlopeFactor;
  desc.RasterizerState.DepthClipEnable =
      createInfo.rasterizerState.enableDepthClip;
  desc.DepthStencilState.DepthEnable =
      createInfo.depthStencilState.enableDepthTest;
  desc.DepthStencilState.DepthWriteMask =
      createInfo.depthStencilState.enableDepthWrite
          ? D3D12_DEPTH_WRITE_MASK_ALL
          : D3D12_DEPTH_WRITE_MASK_ZERO;
  desc.DepthStencilState.DepthFunc =
      convert::compareOpFrom(createInfo.depthStencilState.compareOp);
  desc.DepthStencilState.StencilEnable =
      createInfo.depthStencilState.enableStencilTest;
  desc.DepthStencilState.StencilReadMask =
      createInfo.depthStencilState.compareMask;
  desc.DepthStencilState.StencilWriteMask =
      createInfo.depthStencilState.writeMask;
  desc.DepthStencilState.FrontFace.StencilFailOp = convert::stencilOpFrom(
      createInfo.depthStencilState.frontStencilState.failOp);
  desc.DepthStencilState.FrontFace.StencilDepthFailOp = convert::stencilOpFrom(
      createInfo.depthStencilState.frontStencilState.depthFailOp);
  desc.DepthStencilState.FrontFace.StencilPassOp = convert::stencilOpFrom(
      createInfo.depthStencilState.frontStencilState.passOp);
  desc.DepthStencilState.FrontFace.StencilFunc = convert::compareOpFrom(
      createInfo.depthStencilState.frontStencilState.compareOp);
  desc.DepthStencilState.BackFace = desc.DepthStencilState.FrontFace;
  desc.InputLayout = {elements.data(), static_cast<UINT>(elements.size())};
  desc.PrimitiveTopologyType =
      convert::primitiveTopologyTypeFrom(createInfo.primitiveType);
  desc.NumRenderTargets =
      static_cast<UINT>(createInfo.targetInfo.colorTargetDescriptions.size());
  desc.DSVFormat = createInfo.targetInfo.hasDepthStencilTarget
                       ? convert::textureFormatFrom(
                             createInfo.targetInfo.depthStencilTargetFormat)
                       : DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count =
      convert::sampleCountFrom(createInfo.multiSampleState.sampleCount);

  ComPtr<ID3D12PipelineState> pso;
  HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
  logIfFailed(hr, "D3D12: CreateGraphicsPipelineState failed");
  return makePtr<GraphicsPipeline>(
      createInfo.allocator, createInfo, get(), graphicsRootSignature, pso,
      convert::primitiveTopologyFrom(createInfo.primitiveType));
}

Ptr<gpu::ComputePipeline> Device::createComputePipeline(
    const gpu::ComputePipeline::CreateInfo &createInfo) {
  auto shader = downCast<Shader>(createInfo.computeShader);
  if (!shader) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: create compute pipeline missing shader");
    return nullptr;
  }

  D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
  desc.pRootSignature = computeRootSignature.Get();
  desc.CS = shader->getBytecode();

  ComPtr<ID3D12PipelineState> pso;
  HRESULT hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso));
  logIfFailed(hr, "D3D12: CreateComputePipelineState failed");
  if (FAILED(hr)) {
    return nullptr;
  }

  return makePtr<ComputePipeline>(createInfo.allocator, createInfo, get(),
                                  computeRootSignature, pso);
}

Ptr<gpu::CommandBuffer>
Device::acquireCommandBuffer(const gpu::CommandBuffer::CreateInfo &createInfo) {
  if (deviceLost || !isValid()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: cannot acquire command buffer because device is "
                 "unavailable");
    return nullptr;
  }
  ComPtr<ID3D12CommandAllocator> allocator;
  HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&allocator));
  logIfFailed(hr, "D3D12: CreateCommandAllocator failed");
  if (FAILED(hr)) {
    return nullptr;
  }
  ComPtr<ID3D12GraphicsCommandList> list;
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 allocator.Get(), nullptr, IID_PPV_ARGS(&list));
  logIfFailed(hr, "D3D12: CreateCommandList failed");
  if (FAILED(hr)) {
    return nullptr;
  }
  ID3D12DescriptorHeap *heaps[] = {srvHeap.Get(), samplerHeap.Get()};
  list->SetDescriptorHeaps(2, heaps);
  list->SetGraphicsRootSignature(graphicsRootSignature.Get());
  return makePtr<CommandBuffer>(createInfo.allocator, createInfo, get(),
                                allocator, list);
}

Ptr<gpu::Texture>
Device::acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) {
  if (deviceLost || !commandBuffer) {
    return nullptr;
  }
  // Descriptor tables are frame-local. Upload command buffers can be created
  // while recording draws, so resetting from acquireCommandBuffer would
  // invalidate descriptors already referenced by the active render command
  // list.
  resetTransientDescriptors();
  if (!swapchain) {
    createSwapchain();
  } else if (window) {
    int w = 0;
    int h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    if (static_cast<Uint32>(max(1, w)) != width ||
        static_cast<Uint32>(max(1, h)) != height) {
      waitForGpuIdle();
      createSwapchain();
    }
  }
  if (!swapchain || swapchainTextures.empty()) {
    return nullptr;
  }
  currentBackBuffer = swapchain->GetCurrentBackBufferIndex();
  if (currentBackBuffer >= swapchainTextures.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: invalid swapchain back buffer index");
    return nullptr;
  }
  downCast<CommandBuffer>(commandBuffer)->markUsesSwapchain();
  return swapchainTextures[currentBackBuffer];
}

void Device::submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) {
  if (deviceLost || !commandBuffer) {
    return;
  }
  auto cb = downCast<CommandBuffer>(commandBuffer);
  if (cb->usesSwapchain()) {
    auto texture = downCast<Texture>(swapchainTextures[currentBackBuffer]);
    transition(cb->getNative(), texture.get(), D3D12_RESOURCE_STATE_PRESENT);
  }
  cb->close();
  ID3D12CommandList *lists[] = {cb->getNative()};
  commandQueue->ExecuteCommandLists(1, lists);
  if (cb->usesSwapchain() && swapchain) {
    HRESULT hr = swapchain->Present(1, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
      markDeviceLost("D3D12: device removed during Present",
                     device->GetDeviceRemovedReason());
      return;
    } else {
      logIfFailed(hr, "D3D12: Present failed");
    }
  }
  signalAndWait();
}

void Device::waitForGpuIdle() {
  if (!deviceLost && commandQueue && fence) {
    signalAndWait();
  }
}

void Device::disableRayTracing(const char *reason) {
  if (reason) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s; ray tracing disabled",
                reason);
  }
  rayTracingRootSignature.Reset();
  device5.Reset();
  rayTracingSupported = false;
  rayQuerySupported = false;
}

void Device::markDeviceLost(const char *context, HRESULT hr) {
  if (deviceLost) {
    return;
  }
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s (HRESULT 0x%08X)", context,
               static_cast<unsigned int>(hr));
  deviceLost = true;
  disableRayTracing("D3D12: device is lost");
  destroySwapchain();
}

void Device::signalAndWait() {
  if (deviceLost || !commandQueue || !fence || !fenceEvent) {
    return;
  }
  const UINT64 value = ++fenceValue;
  HRESULT hr = commandQueue->Signal(fence.Get(), value);
  if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
    markDeviceLost("D3D12: device removed during queue signal",
                   device->GetDeviceRemovedReason());
    return;
  }
  logIfFailed(hr, "D3D12: queue signal failed");
  if (FAILED(hr)) {
    return;
  }
  if (fence->GetCompletedValue() < value) {
    hr = fence->SetEventOnCompletion(value, static_cast<HANDLE>(fenceEvent));
    logIfFailed(hr, "D3D12: fence SetEventOnCompletion failed");
    if (FAILED(hr)) {
      return;
    }
    WaitForSingleObject(static_cast<HANDLE>(fenceEvent), INFINITE);
  }
}

gpu::TextureFormat Device::getSwapchainFormat() const {
  return convert::textureFormatFrom(swapchainFormat);
}

String Device::getDriver() const {
  return String("direct3d12", getCreateInfo().allocator);
}

void Device::transition(ID3D12GraphicsCommandList *list, Texture *texture,
                        D3D12_RESOURCE_STATES after) {
  if (!texture || texture->getState() == after) {
    return;
  }
  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = texture->getNative();
  barrier.Transition.StateBefore = texture->getState();
  barrier.Transition.StateAfter = after;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  list->ResourceBarrier(1, &barrier);
  texture->setState(after);
}

void Device::transition(ID3D12GraphicsCommandList *list, Buffer *buffer,
                        D3D12_RESOURCE_STATES after) {
  if (!buffer || buffer->getState() == after) {
    return;
  }
  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = buffer->getNative();
  barrier.Transition.StateBefore = buffer->getState();
  barrier.Transition.StateAfter = after;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  list->ResourceBarrier(1, &barrier);
  buffer->setState(after);
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::cpuHandle(ID3D12DescriptorHeap *heap,
                                              UINT descriptorSize,
                                              UINT index) const {
  auto handle = heap->GetCPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<SIZE_T>(index) * descriptorSize;
  return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Device::gpuHandle(ID3D12DescriptorHeap *heap,
                                              UINT descriptorSize,
                                              UINT index) const {
  auto handle = heap->GetGPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<UINT64>(index) * descriptorSize;
  return handle;
}

CpuGpuDescriptor Device::allocateSrvDescriptor() {
  const UINT index = srvUsed++;
  return {cpuHandle(srvCpuHeap.Get(), srvDescriptorSize, index), {}};
}

CpuGpuDescriptor Device::allocateSamplerDescriptor() {
  const UINT index = samplerUsed++;
  return {cpuHandle(samplerCpuHeap.Get(), samplerDescriptorSize, index), {}};
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::allocateRtvDescriptor() {
  return cpuHandle(rtvHeap.Get(), rtvDescriptorSize, rtvUsed++);
}

D3D12_CPU_DESCRIPTOR_HANDLE Device::allocateDsvDescriptor() {
  return cpuHandle(dsvHeap.Get(), dsvDescriptorSize, dsvUsed++);
}

CpuGpuDescriptor Device::allocateTransientSrvDescriptor() {
  const UINT index = transientSrvUsed++;
  return {cpuHandle(srvHeap.Get(), srvDescriptorSize, index),
          gpuHandle(srvHeap.Get(), srvDescriptorSize, index)};
}

CpuGpuDescriptor Device::allocateTransientSamplerDescriptor() {
  const UINT index = transientSamplerUsed++;
  return {cpuHandle(samplerHeap.Get(), samplerDescriptorSize, index),
          gpuHandle(samplerHeap.Get(), samplerDescriptorSize, index)};
}

void Device::resetTransientDescriptors() {
  transientSrvUsed = TransientSrvBase;
  transientSamplerUsed = TransientSamplerBase;
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
