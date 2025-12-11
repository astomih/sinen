#ifdef _WIN32
#include "d3d12u_renderer.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include <windows.h>

#include <iostream>

#include <graphics/rhi/rhi.hpp>

namespace sinen::rhi {
#define QUERY_INTERFACE(from, to)                                              \
  from->QueryInterface(__uuidof(decltype(to)), (void **)&to)

D3d12uRenderer::D3d12uRenderer(Allocator *allocator)
    : allocator(allocator), device(nullptr) {}

D3d12uRenderer::~D3d12uRenderer() {
  samplerDescriptorHeap.heap->Release();
  srvDescriptorHeap.heap->Release();
  imguiDescriptorHeap.heap->Release();
  dsvDescriptorHeap.heap->Release();
  rtvDescriptorHeap.heap->Release();

  this->commandQueue->Release();
#ifdef _DEBUG
  this->d3d12Debug3->Release();
  this->d3d12Debug->Release();
#endif
  device->Release();
}

void D3d12uRenderer::Initialize(void *window) {
  auto sdlWindow = static_cast<SDL_Window *>(window);
  pWindow = sdlWindow;
  SDL_GetWindowSize(sdlWindow, &width, &height);
  hWindow = static_cast<HWND>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow),
                             SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

  PrepareDevice();
  PrepareCommandQueue();
  PrepareDescriptorHeap();
  PrepareSwapChain();
  PrepareRenderTargetView();
  PrepareCommandAllocator();

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer bindings
  ImGui_ImplSDL3_InitForD3D(sdlWindow);
  {

    DescriptorHandle fontDescriptor = {};
    auto info = &this->imguiDescriptorHeap;
    auto desc = info->heap->GetDesc();
    fontDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    fontDescriptor.hCPU = info->heap->GetCPUDescriptorHandleForHeapStart();
    fontDescriptor.hCPU.ptr += info->handleSize * info->usedIndex;
    fontDescriptor.hGPU.ptr = 0;
    if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
      fontDescriptor.hGPU = info->heap->GetGPUDescriptorHandleForHeapStart();
      fontDescriptor.hGPU.ptr += info->handleSize * info->usedIndex;
    }
    info->usedIndex++;
    ImGui_ImplDX12_Init(device, 2, GetSwapchainFormat(),
                        imguiDescriptorHeap.heap, fontDescriptor.hCPU,
                        fontDescriptor.hGPU);
  }

  frameIndex = this->swapChain->GetCurrentBackBufferIndex();

  PrepareTexture();
}

void D3d12uRenderer::ProcessEvent(void *event) {
  ImGui_ImplSDL3_ProcessEvent(static_cast<SDL_Event *>(event));
}

void D3d12uRenderer::BeginFrame() {
  frameInfo[frameIndex].commandAllocator->Reset();

  auto commandList = CreateCommandList();
  auto renderTarget = GetSwapchainBufferResource();

  auto barrierToRT = D3D12_RESOURCE_BARRIER{
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
      .Transition = {
          .pResource = renderTarget,
          .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
          .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
          .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
      }};

  commandList->ResourceBarrier(1, &barrierToRT);

  commandList->SetPipelineState(pipelineState);
  D3D12_VIEWPORT viewport{
      .TopLeftX = 0,
      .TopLeftY = 0,
      .Width = static_cast<float>(width),
      .Height = static_cast<float>(height),
      .MinDepth = 0.0f,
      .MaxDepth = 1.0f,
  };
  commandList->RSSetViewports(1, &viewport);
  D3D12_RECT scissorRect{
      .left = 0,
      .top = 0,
      .right = width,
      .bottom = height,
  };
  commandList->RSSetScissorRects(1, &scissorRect);

  auto rtvHandle = GetSwapchainBufferDescriptor();
  commandList->OMSetRenderTargets(1, &rtvHandle.hCPU, FALSE, nullptr);

  const float clearColor[] = {0.f, 0.f, 0.f, 1.0f};
  commandList->ClearRenderTargetView(rtvHandle.hCPU, clearColor, 0, nullptr);
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
  commandList->SetGraphicsRootSignature(rootSignature);
  ID3D12DescriptorHeap *heaps[] = {
      GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
      GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
  };
  commandList->SetDescriptorHeaps(_countof(heaps), heaps);
  commandList->SetGraphicsRootDescriptorTable(0, textureDescriptor.hGPU);
  commandList->SetGraphicsRootDescriptorTable(1, samplerDescriptor.hGPU);
  commandList->DrawInstanced(6, 1, 0, 0);
  heaps[0] = this->imguiDescriptorHeap.heap;
  commandList->SetDescriptorHeaps(_countof(heaps), heaps);

  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplDX12_NewFrame();
  ImGui::NewFrame();
  for (auto &callBack : this->guiCallBacks) {
    callBack();
  }
  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

  D3D12_RESOURCE_BARRIER barrierToPresent{
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
      .Transition = {
          .pResource = renderTarget,
          .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
          .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
          .StateAfter = D3D12_RESOURCE_STATE_PRESENT,
      }};
  commandList->ResourceBarrier(1, &barrierToPresent);

  commandList->Close();

  Submit(commandList);
  Present(1);
}

void D3d12uRenderer::EndFrame() {}

void D3d12uRenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {
  this->guiCallBacks.push_back(callBack);
}

ID3D12Resource1 *
D3d12uRenderer::CreateBuffer(D3D12_RESOURCE_DESC desc,
                             D3D12_HEAP_PROPERTIES heapProperties) {
  ID3D12Resource1 *buffer;
  this->device->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));
  return buffer;
}

ID3D12Resource1 *D3d12uRenderer::CreateTexture(const void *data, size_t size,
                                               int width, int height) {
  ID3D12Resource1 *texture;
  D3D12_RESOURCE_DESC desc{
      .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
      .Alignment = 0,
      .Width = static_cast<UINT64>(width),
      .Height = static_cast<UINT>(height),
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
      .Flags = D3D12_RESOURCE_FLAG_NONE,
  };
  D3D12_HEAP_PROPERTIES heapProperties{
      .Type = D3D12_HEAP_TYPE_DEFAULT,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 1,
      .VisibleNodeMask = 1,
  };
  HRESULT result = this->device->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));
  if (FAILED(result)) {
    std::cout << "Failed to create texture" << std::endl;
    return nullptr;
  }

  // Upload heap
  D3D12_HEAP_PROPERTIES uploadHeapProperties{
      .Type = D3D12_HEAP_TYPE_UPLOAD,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 1,
      .VisibleNodeMask = 1,
  };
  // Search requred size
  UINT64 uploadBufferRequiredSize = 0;
  UINT64 uploadOffset = 0;
  int mipmapCount = 1;
  std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(mipmapCount);
  std::vector<UINT> numRows(mipmapCount);
  std::vector<UINT64> rowSizeInByte(mipmapCount);

  device->GetCopyableFootprints(
      &desc, 0, mipmapCount, uploadOffset, footprints.data(), numRows.data(),
      rowSizeInByte.data(), &uploadBufferRequiredSize);

  D3D12_RESOURCE_DESC uploadDesc{
      .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = 0,
      .Width = uploadBufferRequiredSize,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE,
  };
  ID3D12Resource1 *staging;
  result = this->device->CreateCommittedResource(
      &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &uploadDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&staging));
  if (FAILED(result)) {
    std::cout << "Failed to create upload buffer" << std::endl;
    return nullptr;
  }

  void *mappedData;
  staging->Map(0, nullptr, &mappedData);
  memcpy(mappedData, data, size);
  staging->Unmap(0, nullptr);

  auto commandList = CreateCommandList();

  // Copy texture region
  D3D12_TEXTURE_COPY_LOCATION dstLocation{
      .pResource = texture,
      .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
      .SubresourceIndex = 0,
  };
  D3D12_TEXTURE_COPY_LOCATION srcLocation{
      .pResource = staging,
      .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
      .PlacedFootprint = footprints[0]};
  commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  // Barrier
  D3D12_RESOURCE_BARRIER barrier{
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
      .Transition =
          {
              .pResource = texture,
              .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
              .StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
              .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
          },
  };
  commandList->ResourceBarrier(1, &barrier);
  commandList->Close();
  Submit(commandList);
  commandQueue->Signal(frameFence, frameInfo[frameIndex].fenceValue);
  commandQueue->Wait(frameFence, frameInfo[frameIndex].fenceValue);
  commandList->Release();

  if (!texture) {
    std::cout << "Failed to create texture" << std::endl;
    return nullptr;
  }
  return texture;
}

D3d12uRenderer::DescriptorHandle
D3d12uRenderer::CreateSampler(const D3D12_SAMPLER_DESC &desc) {
  DescriptorHandle descriptor =
      AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  device->CreateSampler(&desc, descriptor.hCPU);
  return descriptor;
}

ID3D12RootSignature *D3d12uRenderer::CreateRootSignature(ID3DBlob *signature) {
  ID3D12RootSignature *rootSignature;
  HRESULT hr = device->CreateRootSignature(0, signature->GetBufferPointer(),
                                           signature->GetBufferSize(),
                                           IID_PPV_ARGS(&rootSignature));
  if (FAILED(hr)) {
    signature->Release();
    std::cout << "Failed to create root signature" << std::endl;
    return nullptr;
  }
  return rootSignature;
}

ID3D12PipelineState *D3d12uRenderer::CreateGraphicsPipelineState(
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc) {
  ID3D12PipelineState *pso;
  HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
  return pso;
}

ID3D12GraphicsCommandList *D3d12uRenderer::CreateCommandList() {
  ID3D12GraphicsCommandList *commandList;

  HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                         frameInfo[frameIndex].commandAllocator,
                                         nullptr, IID_PPV_ARGS(&commandList));
  if (FAILED(hr)) {
    std::cout << "Failed to create command list" << std::endl;
    return nullptr;
  }
  return commandList;
}

D3d12uRenderer::DescriptorHandle D3d12uRenderer::CreateShaderResourceView(
    ID3D12Resource1 *res, D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc) {
  DescriptorHandle srvDescriptor =
      AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  device->CreateShaderResourceView(res, &srvDesc, srvDescriptor.hCPU);
  return srvDescriptor;
}

D3d12uRenderer::DescriptorHandle
D3d12uRenderer::GetSwapchainBufferDescriptor() {
  return frameInfo[frameIndex].rtvDescriptor;
}

ID3D12Resource1 *D3d12uRenderer::GetSwapchainBufferResource() {
  return frameInfo[frameIndex].targetBuffer;
}

void D3d12uRenderer::Submit(ID3D12CommandList *const commandList) {
  commandQueue->ExecuteCommandLists(1, &commandList);
}

void D3d12uRenderer::Present(UINT syncInterval, UINT flags) {
  if (this->swapChain) {
    swapChain->Present(syncInterval, flags);
    const UINT64 currentFenceValue = frameInfo[frameIndex].fenceValue;
    commandQueue->Signal(frameFence, currentFenceValue);

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    const UINT64 expectValue = frameInfo[frameIndex].fenceValue;
    if (frameFence->GetCompletedValue() < expectValue) {
      frameFence->SetEventOnCompletion(expectValue, waitFence);
      WaitForSingleObjectEx(waitFence, INFINITE, FALSE);
    }
    frameInfo[frameIndex].fenceValue = currentFenceValue + 1;
  }
}

void D3d12uRenderer::PrepareDevice() {
#ifdef _DEBUG
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
    d3d12Debug->EnableDebugLayer();
    UINT dxgiFlags = DXGI_CREATE_FACTORY_DEBUG;
  }
  QUERY_INTERFACE(d3d12Debug, d3d12Debug3);
  if (d3d12Debug3) {
    d3d12Debug3->SetEnableGPUBasedValidation(TRUE);
  }
#endif

  UINT dxgiFlags = 0;
  CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&dxgiFactory));

  UINT adapterIndex = 0;
  IDXGIAdapter1 *adapter = nullptr;
  while (DXGI_ERROR_NOT_FOUND !=
         this->dxgiFactory->EnumAdapters1(adapterIndex, &adapter)) {
    DXGI_ADAPTER_DESC1 desc1{};
    adapter->GetDesc1(&desc1);
    ++adapterIndex;
    if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }
    // Use able to D3D12?
    HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2,
                                   __uuidof(ID3D12Device), nullptr);
    if (SUCCEEDED(hr)) {
      QUERY_INTERFACE(adapter, this->adapter);
      break;
    }
  }
  D3D12CreateDevice(this->adapter, D3D_FEATURE_LEVEL_12_2,
                    IID_PPV_ARGS(&this->device));
}

void D3d12uRenderer::PrepareCommandQueue() {
  D3D12_COMMAND_QUEUE_DESC queueDesc{.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     .Priority = 0,
                                     .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                                     .NodeMask = 0};
  this->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
}

void D3d12uRenderer::PrepareDescriptorHeap() {
  D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                        .NumDescriptors = 32,
                                        .Flags =
                                            D3D12_DESCRIPTOR_HEAP_FLAG_NONE};
  this->device->CreateDescriptorHeap(
      &rtvDesc, IID_PPV_ARGS(&this->rtvDescriptorHeap.heap));
  this->rtvDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
      .NumDescriptors = 32,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
  };
  this->device->CreateDescriptorHeap(&dsvHeapDesc,
                                     IID_PPV_ARGS(&dsvDescriptorHeap.heap));
  this->dsvDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
      .NumDescriptors = 2048,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};
  this->device->CreateDescriptorHeap(&srvHeapDesc,
                                     IID_PPV_ARGS(&srvDescriptorHeap.heap));
  this->device->CreateDescriptorHeap(&srvHeapDesc,
                                     IID_PPV_ARGS(&imguiDescriptorHeap.heap));
  this->srvDescriptorHeap.handleSize = device->CreateDescriptorHeap(
      &srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap.heap));
  this->imguiDescriptorHeap.handleSize = device->CreateDescriptorHeap(
      &srvHeapDesc, IID_PPV_ARGS(&imguiDescriptorHeap.heap));

  D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
      .NumDescriptors = 2048,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};
  this->device->CreateDescriptorHeap(&samplerHeapDesc,
                                     IID_PPV_ARGS(&samplerDescriptorHeap.heap));
  this->samplerDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(
          D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

D3d12uRenderer::DescriptorHandle
D3d12uRenderer::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) {
  DescriptorHandle handle = {};
  auto info = GetDescriptorHeapInfo(type);
  if (!info->handles.empty()) {
    handle = info->handles.back();
    info->handles.pop_back();
    return handle;
  }
  auto desc = info->heap->GetDesc();
  handle.type = type;
  handle.hCPU = info->heap->GetCPUDescriptorHandleForHeapStart();
  handle.hCPU.ptr += info->handleSize * info->usedIndex;
  handle.hGPU.ptr = 0;
  if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
    handle.hGPU = info->heap->GetGPUDescriptorHandleForHeapStart();
    handle.hGPU.ptr += info->handleSize * info->usedIndex;
  }
  info->usedIndex++;
  return handle;
}

void D3d12uRenderer::DeallocateDescriptor(DescriptorHandle descriptor) {
  auto info = GetDescriptorHeapInfo(descriptor.type);
  info->handles.push_back(descriptor);
}

ID3D12DescriptorHeap *
D3d12uRenderer::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
  auto info = GetDescriptorHeapInfo(type);
  return info->heap;
}

D3d12uRenderer::DescriptorHeapInfo *
D3d12uRenderer::GetDescriptorHeapInfo(D3D12_DESCRIPTOR_HEAP_TYPE type) {
  switch (type) {
  case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
    return &srvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
    return &rtvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
    return &dsvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
    return &samplerDescriptorHeap;
  default:
    return nullptr;
  }
}

void D3d12uRenderer::PrepareCommandAllocator() {
  waitFence = CreateEvent(NULL, FALSE, FALSE, NULL);
  device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFence));

  for (UINT i = 0; i < frameCount; i++) {
    auto &frame = frameInfo[i];
    frame.fenceValue = 0;
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                   IID_PPV_ARGS(&frame.commandAllocator));
  }
}

void D3d12uRenderer::PrepareSwapChain() {
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
      .Width = static_cast<UINT>(width),
      .Height = static_cast<UINT>(height),
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = frameCount,
      .Scaling = DXGI_SCALING_STRETCH,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD};
  IDXGISwapChain1 *pSwapChain1 = nullptr;
  dxgiFactory->CreateSwapChainForHwnd(commandQueue, hWindow, &swapChainDesc,
                                      nullptr, nullptr, &pSwapChain1);
  QUERY_INTERFACE(pSwapChain1, swapChain);
  dxgiFactory->MakeWindowAssociation(hWindow, DXGI_MWA_NO_ALT_ENTER);
}

void D3d12uRenderer::PrepareRenderTargetView() {
  for (UINT i = 0; i < frameCount; i++) {
    ID3D12Resource1 *rt;
    swapChain->GetBuffer(i, IID_PPV_ARGS(&rt));

    auto descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    device->CreateRenderTargetView(rt, nullptr, descriptor.hCPU);
    frameInfo[i].rtvDescriptor = descriptor;
    frameInfo[i].targetBuffer = rt;
  }
}

void D3d12uRenderer::PrepareTexture() {
  // Load texture from SDL
  SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");

  std::vector<uint8_t> data;
  data.resize(surface->w * surface->h * 4);

  for (int y = 0; y < surface->h; ++y) {
    for (int x = 0; x < surface->w; ++x) {
      auto pixel =
          static_cast<uint32_t *>(surface->pixels) + y * surface->w + x;
      auto r = (*pixel & 0x00FF0000) >> 16;
      auto g = (*pixel & 0x0000FF00) >> 8;
      auto b = (*pixel & 0x000000FF);
      auto a = (*pixel & 0xFF000000) >> 24;
      auto index = (y * surface->w + x) * 4;
      data[index + 0] = r;
      data[index + 1] = g;
      data[index + 2] = b;
      data[index + 3] = a;
    }
  }
  texture = CreateTexture(data.data(), data.size(), surface->w, surface->h);

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
      .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
      .Texture2D = {.MostDetailedMip = 0,
                    .MipLevels = 1,
                    .PlaneSlice = 0,
                    .ResourceMinLODClamp = 0.0f},
  };
  textureDescriptor = CreateShaderResourceView(texture, srvDesc);
  samplerDescriptor =
      CreateSampler({.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                     .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
                     .MinLOD = 0,
                     .MaxLOD = D3D12_FLOAT32_MAX});

  /*
 (-1,  1)  (1,  1)
    +--------+
    |        |
    |        |
    |        |
    +--------+
 (-1, -1)  (1, -1)
  */
  float triangleVertices[] = {
      -1.f, -1.f, 0.f, 0, 0, 1, 1, 1, // position, uv, color
      -1.f, 1.f,  0.f, 0, 1, 1, 1, 1, //
      1.f,  -1.f, 0.f, 1, 0, 1, 1, 1, //
      1.f,  -1.f, 0.f, 1, 0, 0, 0, 1, //
      -1.f, 1.f,  0.f, 0, 1, 1, 0, 0, //
      1.f,  1.f,  0.f, 1, 1, 1, 0, 0, //
  };

  D3D12_HEAP_PROPERTIES uploadHeap{
      .Type = D3D12_HEAP_TYPE_UPLOAD,
      .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
      .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
      .CreationNodeMask = 0,
      .VisibleNodeMask = 0,
  };
  D3D12_RESOURCE_DESC resDesc{.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                              .Alignment = 0,
                              .Width =
                                  sizeof(float) * _countof(triangleVertices),
                              .Height = 1,
                              .DepthOrArraySize = 1,
                              .MipLevels = 1,
                              .Format = DXGI_FORMAT_UNKNOWN,
                              .SampleDesc = {.Count = 1, .Quality = 0},
                              .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                              .Flags = D3D12_RESOURCE_FLAG_NONE};
  vertexBuffer = CreateBuffer(resDesc, uploadHeap);

  void *mapped = nullptr;
  if (vertexBuffer) {
    vertexBuffer->Map(0, nullptr, &mapped);
    if (mapped) {
      memcpy(mapped, triangleVertices,
             sizeof(float) * _countof(triangleVertices));
      vertexBuffer->Unmap(0, nullptr);
    }
  }
  vertexBufferView = D3D12_VERTEX_BUFFER_VIEW{
      .BufferLocation = vertexBuffer->GetGPUVirtualAddress(),
      .SizeInBytes = sizeof(float) * _countof(triangleVertices),
      .StrideInBytes = sizeof(float) * 8};

  D3D12_DESCRIPTOR_RANGE rangeSrvRanges[] = {{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
      .NumDescriptors = 1,
      .BaseShaderRegister = 0,
      .RegisterSpace = 0,
      .OffsetInDescriptorsFromTableStart = 0,
  }};
  D3D12_DESCRIPTOR_RANGE rangeSamplerRanges[] = {{
      .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
      .NumDescriptors = 1,
      .BaseShaderRegister = 0,
      .RegisterSpace = 0,
      .OffsetInDescriptorsFromTableStart = 0,

  }};
  std::vector<D3D12_ROOT_PARAMETER> rootParameters;
  rootParameters.emplace_back(D3D12_ROOT_PARAMETER{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable = {.NumDescriptorRanges = _countof(rangeSrvRanges),
                          .pDescriptorRanges = rangeSrvRanges},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
  });
  rootParameters.emplace_back(D3D12_ROOT_PARAMETER{
      .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
      .DescriptorTable = {.NumDescriptorRanges = _countof(rangeSamplerRanges),
                          .pDescriptorRanges = rangeSamplerRanges},
      .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
  });

  D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{
      .NumParameters = static_cast<UINT>(rootParameters.size()),
      .pParameters = rootParameters.data(),
      .NumStaticSamplers = 0,
      .pStaticSamplers = nullptr,
      .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

  ID3DBlob *signatureBlob = nullptr;
  ID3DBlob *errorBlob = nullptr;
  D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                              &signatureBlob, &errorBlob);
  rootSignature = CreateRootSignature(signatureBlob);
  D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
      {
          .SemanticName = "POSITION",
          .SemanticIndex = 0,
          .Format = DXGI_FORMAT_R32G32B32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = 0,
          .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0,
      },
      {
          .SemanticName = "TEXCOORD",
          .SemanticIndex = 0,
          .Format = DXGI_FORMAT_R32G32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = sizeof(float) * 3,
          .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0,
      },
      {
          .SemanticName = "COLOR",
          .SemanticIndex = 0,
          .Format = DXGI_FORMAT_R32G32B32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = sizeof(float) * 5,
          .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0,
      },
  };
  D3D12_INPUT_LAYOUT_DESC inputLayout{
      .pInputElementDescs = inputElementDesc,
      .NumElements = _countof(inputElementDesc),
  };
  std::vector<char> vsdata, psdata;
  D3D12_SHADER_BYTECODE vs{
      .pShaderBytecode = vsdata.data(),
      .BytecodeLength = vsdata.size(),
  };
  D3D12_SHADER_BYTECODE ps{
      .pShaderBytecode = psdata.data(),
      .BytecodeLength = psdata.size(),
  };
  D3D12_BLEND_DESC blendState{
      .AlphaToCoverageEnable = FALSE,
      .IndependentBlendEnable = FALSE,
      .RenderTarget = {
          D3D12_RENDER_TARGET_BLEND_DESC{.BlendEnable = FALSE,
                                         .LogicOpEnable = FALSE,
                                         .SrcBlend = D3D12_BLEND_ONE,
                                         .DestBlend = D3D12_BLEND_ZERO,
                                         .BlendOp = D3D12_BLEND_OP_ADD,
                                         .SrcBlendAlpha = D3D12_BLEND_ONE,
                                         .DestBlendAlpha = D3D12_BLEND_ZERO,
                                         .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                                         .LogicOp = D3D12_LOGIC_OP_NOOP,
                                         .RenderTargetWriteMask =
                                             D3D12_COLOR_WRITE_ENABLE_ALL},
      }};

  D3D12_RASTERIZER_DESC rasterizerState{
      .FillMode = D3D12_FILL_MODE_SOLID,
      .CullMode = D3D12_CULL_MODE_NONE,
      .FrontCounterClockwise = TRUE,
      .DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
      .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
      .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
      .DepthClipEnable = TRUE,
      .MultisampleEnable = FALSE,
      .AntialiasedLineEnable = FALSE,
      .ForcedSampleCount = 0,
      .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};
  const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {
      .StencilFailOp = D3D12_STENCIL_OP_KEEP,
      .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
      .StencilPassOp = D3D12_STENCIL_OP_KEEP,
      .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS};
  D3D12_DEPTH_STENCIL_DESC depthStencilState{
      .DepthEnable = FALSE,
      .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
      .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
      .StencilEnable = FALSE,
      .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
      .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
      .FrontFace = defaultStencilOp,
      .BackFace = defaultStencilOp};

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout = inputLayout;
  psoDesc.pRootSignature = rootSignature;
  psoDesc.VS = vs;
  psoDesc.PS = ps;
  psoDesc.RasterizerState = rasterizerState;
  psoDesc.BlendState = blendState;
  psoDesc.DepthStencilState = depthStencilState;
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = GetSwapchainFormat();
  psoDesc.SampleDesc.Count = 1;
  psoDesc.SampleDesc.Quality = 0;
  pipelineState = CreateGraphicsPipelineState(psoDesc);
}

// namespace paranoixa
} // namespace sinen::rhi
#endif // _WIN32
