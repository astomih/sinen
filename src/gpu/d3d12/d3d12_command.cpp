#include "d3d12_command.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_convert.hpp"
#include "d3d12_device.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstring>

namespace sinen::gpu::d3d12 {
namespace {
D3D12_RESOURCE_DESC bufferDesc(UINT64 size) {
  D3D12_RESOURCE_DESC desc{};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  return desc;
}

D3D12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE type) {
  D3D12_HEAP_PROPERTIES props{};
  props.Type = type;
  props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  props.CreationNodeMask = 1;
  props.VisibleNodeMask = 1;
  return props;
}

UINT64 alignTo(UINT64 value, UINT64 alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

UINT bytesPerPixel(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8_UNORM:
    return 1;
  case TextureFormat::R8G8_UNORM:
    return 2;
  case TextureFormat::R8G8B8A8_UNORM:
  case TextureFormat::B8G8R8A8_UNORM:
    return 4;
  case TextureFormat::R32G32B32A32_FLOAT:
    return 16;
  default:
    return 4;
  }
}
} // namespace

CommandBuffer::CommandBuffer(const CreateInfo &createInfo, Ptr<Device> device,
                             ComPtr<ID3D12CommandAllocator> allocator,
                             ComPtr<ID3D12GraphicsCommandList> commandList)
    : gpu::CommandBuffer(createInfo), device(device), allocator(allocator),
      commandList(commandList) {}

void CommandBuffer::close() {
  if (!closed) {
    commandList->Close();
    closed = true;
  }
}

Ptr<gpu::CopyPass> CommandBuffer::beginCopyPass() {
  return makePtr<CopyPass>(getCreateInfo().allocator, this);
}

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass>) {}

Ptr<gpu::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  Array<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs(getCreateInfo().allocator);
  for (const auto &info : infos) {
    auto texture = downCast<Texture>(info.texture);
    if (!texture) {
      continue;
    }
    device->transition(commandList.Get(), texture.get(),
                       D3D12_RESOURCE_STATE_RENDER_TARGET);
    rtvs.push_back(texture->getRtv());
    if (info.loadOp == LoadOp::Clear) {
      const float color[4] = {r, g, b, a};
      commandList->ClearRenderTargetView(texture->getRtv(), color, 0, nullptr);
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE dsv{};
  D3D12_CPU_DESCRIPTOR_HANDLE *dsvPtr = nullptr;
  if (depthStencilInfo.texture) {
    auto depth = downCast<Texture>(depthStencilInfo.texture);
    device->transition(commandList.Get(), depth.get(),
                       D3D12_RESOURCE_STATE_DEPTH_WRITE);
    dsv = depth->getDsv();
    dsvPtr = &dsv;
    if (depthStencilInfo.loadOp == LoadOp::Clear) {
      commandList->ClearDepthStencilView(
          dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
          depthStencilInfo.clearDepth, depthStencilInfo.clearStencil, 0,
          nullptr);
    }
  }

  if (!rtvs.empty()) {
    commandList->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(),
                                    FALSE, dsvPtr);
  }
  return makePtr<RenderPass>(getCreateInfo().allocator, this, infos,
                             depthStencilInfo);
}

void CommandBuffer::endRenderPass(Ptr<gpu::RenderPass>) {}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::uploadUniform(const void *data,
                                                       size_t size) {
  if (!data || size == 0) {
    return 0;
  }
  const UINT64 uploadSize = alignTo(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
  ComPtr<ID3D12Resource> resource;
  auto heap = heapProperties(D3D12_HEAP_TYPE_UPLOAD);
  auto desc = bufferDesc(uploadSize);
  HRESULT hr = device->getNative()->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&resource));
  if (FAILED(hr)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: uniform upload allocation failed");
    return 0;
  }
  void *mapped = nullptr;
  resource->Map(0, nullptr, &mapped);
  std::memcpy(mapped, data, size);
  resource->Unmap(0, nullptr);
  auto address = resource->GetGPUVirtualAddress();
  keepAlive(resource);
  return address;
}

void CommandBuffer::pushVertexUniformData(UInt32 slot, const void *data,
                                          size_t size) {
  if (slot < vertexUniforms.size()) {
    vertexUniforms[slot] = uploadUniform(data, size);
  }
}

void CommandBuffer::pushFragmentUniformData(UInt32 slot, const void *data,
                                            size_t size) {
  if (slot < fragmentUniforms.size()) {
    fragmentUniforms[slot] = uploadUniform(data, size);
  }
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::vertexUniform(UInt32 slot) const {
  return slot < vertexUniforms.size() ? vertexUniforms[slot] : 0;
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::fragmentUniform(UInt32 slot) const {
  return slot < fragmentUniforms.size() ? fragmentUniforms[slot] : 0;
}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool) {
  auto source = downCast<TransferBuffer>(src.transferBuffer);
  auto target = downCast<Buffer>(dst.buffer);
  auto list = commandBuffer->getNative();
  commandBuffer->getDevice()->transition(list, target.get(),
                                         D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyBufferRegion(target->getNative(), dst.offset, source->getNative(),
                         src.offset, dst.size);
  commandBuffer->getDevice()->transition(
      list, target.get(), convert::bufferStateFrom(target->getCreateInfo().usage));
}

void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  auto source = downCast<Buffer>(src.buffer);
  auto target = downCast<TransferBuffer>(dst.transferBuffer);
  auto list = commandBuffer->getNative();
  commandBuffer->getDevice()->transition(list, source.get(),
                                         D3D12_RESOURCE_STATE_COPY_SOURCE);
  list->CopyBufferRegion(target->getNative(), dst.offset, source->getNative(),
                         src.offset, src.size);
}

void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool) {
  auto source = downCast<TransferBuffer>(src.transferBuffer);
  auto target = downCast<Texture>(dst.texture);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();

  D3D12_RESOURCE_DESC textureDesc = target->getNative()->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT numRows = 0;
  UINT64 rowSize = 0;
  UINT64 totalSize = 0;
  const UINT dstSubresource =
      dst.mipLevel + dst.layer * target->getCreateInfo().numLevels;
  device->getNative()->GetCopyableFootprints(&textureDesc, dstSubresource, 1,
                                             0, &footprint, &numRows, &rowSize,
                                             &totalSize);

  ComPtr<ID3D12Resource> upload;
  auto heap = heapProperties(D3D12_HEAP_TYPE_UPLOAD);
  auto desc = bufferDesc(totalSize);
  device->getNative()->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&upload));

  const UINT srcRowPitch = dst.width * bytesPerPixel(target->getCreateInfo().format);
  auto *srcBytes = static_cast<const uint8_t *>(source->map(false)) + src.offset;
  void *mapped = nullptr;
  upload->Map(0, nullptr, &mapped);
  auto *dstBytes = static_cast<uint8_t *>(mapped) + footprint.Offset;
  for (UINT row = 0; row < numRows; ++row) {
    std::memcpy(dstBytes + row * footprint.Footprint.RowPitch,
                srcBytes + row * srcRowPitch,
                std::min<UINT64>(srcRowPitch, rowSize));
  }
  upload->Unmap(0, nullptr);
  commandBuffer->keepAlive(upload);

  D3D12_TEXTURE_COPY_LOCATION sourceLocation{};
  sourceLocation.pResource = upload.Get();
  sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  sourceLocation.PlacedFootprint = footprint;

  D3D12_TEXTURE_COPY_LOCATION targetLocation{};
  targetLocation.pResource = target->getNative();
  targetLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  targetLocation.SubresourceIndex = dstSubresource;

  D3D12_BOX box{};
  box.left = 0;
  box.top = 0;
  box.front = 0;
  box.right = dst.width;
  box.bottom = dst.height;
  box.back = dst.depth;

  device->transition(list, target.get(), D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyTextureRegion(&targetLocation, dst.x, dst.y, dst.z, &sourceLocation,
                          &box);
  device->transition(list, target.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {
  auto source = downCast<Texture>(src.texture);
  auto target = downCast<TransferBuffer>(dst.transferBuffer);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();

  D3D12_RESOURCE_DESC textureDesc = source->getNative()->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  footprint.Offset = dst.offset;
  UINT numRows = 0;
  UINT64 rowSize = 0;
  UINT64 totalSize = 0;
  const UINT srcSubresource =
      src.mipLevel + src.layer * source->getCreateInfo().numLevels;
  device->getNative()->GetCopyableFootprints(&textureDesc, srcSubresource, 1,
                                             dst.offset, &footprint, &numRows,
                                             &rowSize, &totalSize);

  D3D12_TEXTURE_COPY_LOCATION sourceLocation{};
  sourceLocation.pResource = source->getNative();
  sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  sourceLocation.SubresourceIndex = srcSubresource;

  D3D12_TEXTURE_COPY_LOCATION targetLocation{};
  targetLocation.pResource = target->getNative();
  targetLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  targetLocation.PlacedFootprint = footprint;

  D3D12_BOX box{};
  box.left = src.x;
  box.top = src.y;
  box.front = src.z;
  box.right = src.x + src.width;
  box.bottom = src.y + src.height;
  box.back = src.z + src.depth;

  device->transition(list, source.get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  list->CopyTextureRegion(&targetLocation, 0, 0, 0, &sourceLocation, &box);
}

void CopyPass::copyTexture(const TextureLocation &src, const TextureLocation &dst,
                           UInt32 width, UInt32 height, UInt32 depth, bool) {
  auto source = downCast<Texture>(src.texture);
  auto target = downCast<Texture>(dst.texture);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();

  D3D12_TEXTURE_COPY_LOCATION sourceLocation{};
  sourceLocation.pResource = source->getNative();
  sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  sourceLocation.SubresourceIndex =
      src.mipLevel + src.layer * source->getCreateInfo().numLevels;
  D3D12_TEXTURE_COPY_LOCATION targetLocation{};
  targetLocation.pResource = target->getNative();
  targetLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  targetLocation.SubresourceIndex =
      dst.mipLevel + dst.layer * target->getCreateInfo().numLevels;
  D3D12_BOX box{src.x, src.y, src.z, src.x + width, src.y + height,
                src.z + depth};

  device->transition(list, source.get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  device->transition(list, target.get(), D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyTextureRegion(&targetLocation, dst.x, dst.y, dst.z, &sourceLocation,
                          &box);
}

RenderPass::RenderPass(CommandBuffer *commandBuffer,
                       const Array<ColorTargetInfo> &,
                       const DepthStencilTargetInfo &)
    : commandBuffer(commandBuffer) {}

void RenderPass::bindGraphicsPipeline(Ptr<gpu::GraphicsPipeline> graphicsPipeline) {
  pipeline = downCast<GraphicsPipeline>(graphicsPipeline);
  auto list = commandBuffer->getNative();
  list->SetGraphicsRootSignature(pipeline->getRootSignature());
  list->SetPipelineState(pipeline->getNative());
  list->IASetPrimitiveTopology(pipeline->getTopology());
}

void RenderPass::bindVertexBuffers(UInt32 slot,
                                   const Array<BufferBinding> &bindings) {
  Array<D3D12_VERTEX_BUFFER_VIEW> views(commandBuffer->getCreateInfo().allocator);
  views.resize(bindings.size());
  for (size_t i = 0; i < bindings.size(); ++i) {
    auto buffer = downCast<Buffer>(bindings[i].buffer);
    commandBuffer->getDevice()->transition(
        commandBuffer->getNative(), buffer.get(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    views[i].BufferLocation = buffer->getNative()->GetGPUVirtualAddress() +
                              bindings[i].offset;
    views[i].SizeInBytes = buffer->getCreateInfo().size - bindings[i].offset;
    const auto &vb = pipeline->getCreateInfo()
                         .vertexInputState.vertexBufferDescriptions[slot + i];
    views[i].StrideInBytes = vb.pitch;
  }
  commandBuffer->getNative()->IASetVertexBuffers(slot,
                                                 static_cast<UINT>(views.size()),
                                                 views.data());
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  auto buffer = downCast<Buffer>(binding.buffer);
  commandBuffer->getDevice()->transition(commandBuffer->getNative(), buffer.get(),
                                         D3D12_RESOURCE_STATE_INDEX_BUFFER);
  D3D12_INDEX_BUFFER_VIEW view{};
  view.BufferLocation = buffer->getNative()->GetGPUVirtualAddress() + binding.offset;
  view.SizeInBytes = buffer->getCreateInfo().size - binding.offset;
  view.Format = convert::indexFormatFrom(indexElementSize);
  commandBuffer->getNative()->IASetIndexBuffer(&view);
}

void RenderPass::bindFragmentSamplers(
    UInt32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  const UInt32 count =
      std::max<UInt32>(1, startSlot + static_cast<UInt32>(bindings.size()));
  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor firstSrv{};
  CpuGpuDescriptor firstSampler{};
  for (UInt32 i = 0; i < count; ++i) {
    auto srv = device->allocateTransientSrvDescriptor();
    auto sampler = device->allocateTransientSamplerDescriptor();
    if (i == 0) {
      firstSrv = srv;
      firstSampler = sampler;
    }
    CpuGpuDescriptor srcSrv = device->getDefaultSrv();
    CpuGpuDescriptor srcSampler = device->getDefaultSampler();
    if (i >= startSlot && i < startSlot + bindings.size()) {
      const auto &binding = bindings[i - startSlot];
      if (binding.texture) {
        auto texture = downCast<Texture>(binding.texture);
        device->transition(commandBuffer->getNative(), texture.get(),
                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        srcSrv = {texture->getSrvCpu(), texture->getSrvGpu()};
      }
      if (binding.sampler) {
        auto samplerObj = downCast<Sampler>(binding.sampler);
        srcSampler = {samplerObj->getCpu(), samplerObj->getGpu()};
      }
    }
    device->getNative()->CopyDescriptorsSimple(
        1, srv.cpu, srcSrv.cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->getNative()->CopyDescriptorsSimple(
        1, sampler.cpu, srcSampler.cpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(4, firstSrv.gpu);
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(5,
                                                             firstSampler.gpu);
}

void RenderPass::bindFragmentSampler(UInt32 startSlot,
                                     const TextureSamplerBinding &binding) {
  Array<TextureSamplerBinding> bindings(commandBuffer->getCreateInfo().allocator);
  bindings.push_back(binding);
  bindFragmentSamplers(startSlot, bindings);
}

void RenderPass::setViewport(const Viewport &viewport) {
  D3D12_VIEWPORT vp{viewport.x, viewport.y, viewport.width, viewport.height,
                    viewport.minDepth, viewport.maxDepth};
  commandBuffer->getNative()->RSSetViewports(1, &vp);
}

void RenderPass::setScissor(Int32 x, Int32 y, Int32 width, Int32 height) {
  D3D12_RECT rect{x, y, x + width, y + height};
  commandBuffer->getNative()->RSSetScissorRects(1, &rect);
}

void RenderPass::bindUniforms() {
  auto list = commandBuffer->getNative();
  if (auto address = commandBuffer->vertexUniform(0)) {
    list->SetGraphicsRootConstantBufferView(0, address);
  }
  if (auto address = commandBuffer->vertexUniform(1)) {
    list->SetGraphicsRootConstantBufferView(1, address);
  }
  if (auto address = commandBuffer->fragmentUniform(0)) {
    list->SetGraphicsRootConstantBufferView(2, address);
  }
  if (auto address = commandBuffer->fragmentUniform(1)) {
    list->SetGraphicsRootConstantBufferView(3, address);
  }
}

void RenderPass::drawPrimitives(UInt32 numVertices, UInt32 numInstances,
                                UInt32 firstVertex, UInt32 firstInstance) {
  bindUniforms();
  commandBuffer->getNative()->DrawInstanced(numVertices, numInstances,
                                            firstVertex, firstInstance);
}

void RenderPass::drawIndexedPrimitives(UInt32 numIndices, UInt32 numInstances,
                                       UInt32 firstIndex, UInt32 vertexOffset,
                                       UInt32 firstInstance) {
  bindUniforms();
  commandBuffer->getNative()->DrawIndexedInstanced(
      numIndices, numInstances, firstIndex, static_cast<INT>(vertexOffset),
      firstInstance);
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
