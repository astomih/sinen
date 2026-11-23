#include "d3d12_command.hpp"

#ifdef SINEN_PLATFORM_WINDOWS

#include "d3d12_convert.hpp"
#include "d3d12_device.hpp"

#include <SDL3/SDL.h>
#include <core/profiler.hpp>

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
                             const std::shared_ptr<CommandContext> &context)
    : gpu::CommandBuffer(createInfo), device(device), context(context),
      commandList(context->list) {}

CommandContext::~CommandContext() {
  for (auto &page : pages) {
    if (page.mapped) {
      page.resource->Unmap(0, nullptr);
    }
  }
}

void CommandBuffer::close() {
  if (!context->closed) {
    commandList->Close();
    context->closed = true;
  }
}

Ptr<gpu::CopyPass> CommandBuffer::beginCopyPass() {
  return makePtr<CopyPass>(getCreateInfo().allocator, this);
}

void CommandBuffer::endCopyPass(Ptr<gpu::CopyPass>) {}

Ptr<gpu::ComputePass> CommandBuffer::beginComputePass(
    const Array<StorageTextureBinding> &,
    const Array<StorageBufferBinding> &storageBuffers) {
  return makePtr<ComputePass>(getCreateInfo().allocator, this, storageBuffers);
}

void CommandBuffer::endComputePass(Ptr<gpu::ComputePass>) {}

Ptr<gpu::RayTracingPass> CommandBuffer::beginRayTracingPass() {
  if (!device->supportsRayTracing()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: ray tracing is not supported by this device");
    return nullptr;
  }
  return makePtr<RayTracingPass>(getCreateInfo().allocator, this);
}

void CommandBuffer::endRayTracingPass(Ptr<gpu::RayTracingPass>) {}

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
    keepAlive(texture->getNative());
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
    keepAlive(depth->getNative());
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

void CommandBuffer::endRenderPass(Ptr<gpu::RenderPass> renderPass) {
  auto pass = downCast<RenderPass>(renderPass);
  if (pass) {
    pass->resolveTargets();
  }
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::uploadUniform(const void *data,
                                                       size_t size) {
  ZoneScopedN("D3D12 uniform upload");
  ComPtr<ID3D12Resource> resource;
  UINT64 offset = 0;
  return uploadData(data, size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                    resource, offset)
             ? resource->GetGPUVirtualAddress() + offset
             : 0;
}

Ptr<gpu::Buffer>
CommandBuffer::createTransientBuffer(const gpu::Buffer::CreateInfo &info,
                                     const void *data) {
  ZoneScopedN("D3D12 transient upload");
  if (info.usage != gpu::BufferUsage::Vertex &&
      info.usage != gpu::BufferUsage::Index) {
    return nullptr;
  }
  ComPtr<ID3D12Resource> resource;
  UINT64 offset = 0;
  if (!uploadData(data, info.size, 16, resource, offset)) {
    return nullptr;
  }
  return makePtr<Buffer>(info.allocator, info, device, resource,
                         D3D12_RESOURCE_STATE_GENERIC_READ, offset, true);
}

bool CommandBuffer::uploadData(const void *data, Size size, UINT64 alignment,
                               ComPtr<ID3D12Resource> &resource,
                               UINT64 &offset) {
  if (!data || size == 0 || context->closed || context->fenceValue != 0) {
    return false;
  }
  constexpr UINT64 pageSize = 1024 * 1024;
  if (size > UINT64_MAX - (alignment - 1)) {
    return false;
  }
  const UINT64 uploadSize = alignTo(static_cast<UINT64>(size), alignment);
  // A context is reset only after its fence completes and its recording wrapper
  // is released. Reuse all of its persistently mapped pages before growing.
  while (context->pageIndex < context->pages.size()) {
    auto &page = context->pages[context->pageIndex];
    offset = alignTo(page.used, alignment);
    if (offset <= page.capacity && uploadSize <= page.capacity - offset) {
      break;
    }
    ++context->pageIndex;
  }
  if (context->pageIndex == context->pages.size()) {
    ZoneScopedN("D3D12 upload page create");
    CommandContext::UploadPage page;
    page.capacity = (std::max)(pageSize, uploadSize);
    auto heap = heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = bufferDesc(page.capacity);
    HRESULT hr = device->getNative()->CreateCommittedResource(
        &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&page.resource));
    if (FAILED(hr)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "D3D12: uniform upload allocation failed");
      return false;
    }
    void *mapped = nullptr;
    const D3D12_RANGE readRange{0, 0};
    hr = page.resource->Map(0, &readRange, &mapped);
    if (FAILED(hr) || !mapped) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "D3D12: uniform upload mapping failed");
      return false;
    }
    page.mapped = static_cast<std::byte *>(mapped);
    context->pages.push_back(std::move(page));
    offset = 0;
  }
  auto &page = context->pages[context->pageIndex];
  std::memcpy(page.mapped + offset, data, size);
  page.used = offset + uploadSize;
  resource = page.resource;
  return true;
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

void CommandBuffer::pushComputeUniformData(UInt32 slot, const void *data,
                                           size_t size) {
  if (slot < computeUniforms.size()) {
    computeUniforms[slot] = uploadUniform(data, size);
  }
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::vertexUniform(UInt32 slot) const {
  return slot < vertexUniforms.size() ? vertexUniforms[slot] : 0;
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::fragmentUniform(UInt32 slot) const {
  return slot < fragmentUniforms.size() ? fragmentUniforms[slot] : 0;
}

D3D12_GPU_VIRTUAL_ADDRESS CommandBuffer::computeUniform(UInt32 slot) const {
  return slot < computeUniforms.size() ? computeUniforms[slot] : 0;
}

void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool) {
  auto source = downCast<TransferBuffer>(src.transferBuffer);
  auto target = downCast<Buffer>(dst.buffer);
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());
  commandBuffer->getDevice()->transition(list, target.get(),
                                         D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyBufferRegion(target->getNative(), dst.offset, source->getNative(),
                         src.offset, dst.size);
  commandBuffer->getDevice()->transition(
      list, target.get(),
      convert::bufferStateFrom(target->getCreateInfo().usage));
}

void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  auto source = downCast<Buffer>(src.buffer);
  auto target = downCast<TransferBuffer>(dst.transferBuffer);
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());
  commandBuffer->getDevice()->transition(list, source.get(),
                                         D3D12_RESOURCE_STATE_COPY_SOURCE);
  list->CopyBufferRegion(target->getNative(), dst.offset, source->getNative(),
                         source->getOffset() + src.offset, src.size);
}

void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool) {
  auto source = downCast<TransferBuffer>(src.transferBuffer);
  auto target = downCast<Texture>(dst.texture);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());

  D3D12_RESOURCE_DESC textureDesc = target->getNative()->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT numRows = 0;
  UINT64 rowSize = 0;
  UINT64 totalSize = 0;
  const UINT dstSubresource =
      dst.mipLevel + dst.layer * target->getCreateInfo().numLevels;
  device->getNative()->GetCopyableFootprints(&textureDesc, dstSubresource, 1, 0,
                                             &footprint, &numRows, &rowSize,
                                             &totalSize);

  ComPtr<ID3D12Resource> upload;
  auto heap = heapProperties(D3D12_HEAP_TYPE_UPLOAD);
  auto desc = bufferDesc(totalSize);
  device->getNative()->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&upload));

  const UINT srcRowPitch =
      dst.width * bytesPerPixel(target->getCreateInfo().format);
  auto *srcBytes =
      static_cast<const uint8_t *>(source->map(false)) + src.offset;
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
  device->transition(list, target.get(),
                     D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

bool CopyPass::copyBufferToTexture(const BufferRegion &src,
                                   const TextureRegion &dst,
                                   UInt32 bytesPerRow) {
  if (!validBufferTextureCopy(src, dst, bytesPerRow))
    return false;
  auto source = downCast<Buffer>(src.buffer);
  auto target = downCast<Texture>(dst.texture);
  if (!source || !target)
    return false;
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());
  D3D12_TEXTURE_COPY_LOCATION from{};
  from.pResource = source->getNative();
  from.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  from.PlacedFootprint.Offset = source->getOffset() + src.offset;
  from.PlacedFootprint.Footprint = {target->getNative()->GetDesc().Format,
                                    dst.width, dst.height, 1, bytesPerRow};
  D3D12_TEXTURE_COPY_LOCATION to{};
  to.pResource = target->getNative();
  to.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  to.SubresourceIndex =
      dst.mipLevel + dst.layer * target->getCreateInfo().numLevels;
  device->transition(list, source.get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  device->transition(list, target.get(), D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyTextureRegion(&to, dst.x, dst.y, dst.z, &from, nullptr);
  device->transition(list, source.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  device->transition(list, target.get(),
                     D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  return true;
}

void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {
  auto source = downCast<Texture>(src.texture);
  auto target = downCast<TransferBuffer>(dst.transferBuffer);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());

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

void CopyPass::copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, UInt32 width,
                           UInt32 height, UInt32 depth, bool) {
  auto source = downCast<Texture>(src.texture);
  auto target = downCast<Texture>(dst.texture);
  auto device = commandBuffer->getDevice();
  auto list = commandBuffer->getNative();
  commandBuffer->keepAlive(source->getNative());
  commandBuffer->keepAlive(target->getNative());

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
  D3D12_BOX box{src.x,         src.y,          src.z,
                src.x + width, src.y + height, src.z + depth};

  device->transition(list, source.get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
  device->transition(list, target.get(), D3D12_RESOURCE_STATE_COPY_DEST);
  list->CopyTextureRegion(&targetLocation, dst.x, dst.y, dst.z, &sourceLocation,
                          &box);
}

RenderPass::RenderPass(CommandBuffer *commandBuffer,
                       const Array<ColorTargetInfo> &colorTargets,
                       const DepthStencilTargetInfo &)
    : commandBuffer(commandBuffer),
      colorTargets(commandBuffer->getCreateInfo().allocator) {
  this->colorTargets = colorTargets;
}

void RenderPass::resolveTargets() {
  auto list = commandBuffer->getNative();
  auto device = commandBuffer->getDevice();
  for (const auto &target : colorTargets) {
    auto source = downCast<Texture>(target.texture);
    auto resolve = downCast<Texture>(target.resolveTexture);
    if (!source || !resolve) {
      continue;
    }
    commandBuffer->keepAlive(source->getNative());
    commandBuffer->keepAlive(resolve->getNative());
    device->transition(list, source.get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
    device->transition(list, resolve.get(), D3D12_RESOURCE_STATE_RESOLVE_DEST);
    list->ResolveSubresource(resolve->getNative(), 0, source->getNative(), 0,
                             source->getNative()->GetDesc().Format);
  }
}

void RenderPass::bindGraphicsPipeline(
    Ptr<gpu::GraphicsPipeline> graphicsPipeline) {
  pipeline = downCast<GraphicsPipeline>(graphicsPipeline);
  commandBuffer->keepAlive(pipeline->getNative());
  commandBuffer->keepAlive(pipeline->getRootSignature());
  auto list = commandBuffer->getNative();
  list->SetGraphicsRootSignature(pipeline->getRootSignature());
  list->SetPipelineState(pipeline->getNative());
  list->IASetPrimitiveTopology(pipeline->getTopology());
}

void RenderPass::bindVertexBuffers(UInt32 slot,
                                   const Array<BufferBinding> &bindings) {
  if (!pipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: bindVertexBuffers called without a pipeline");
    return;
  }

  const auto &vertexBuffers =
      pipeline->getCreateInfo().vertexInputState.vertexBufferDescriptions;
  if (slot >= vertexBuffers.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: vertex buffer slot %u is outside pipeline layout",
                 slot);
    return;
  }

  const size_t viewCount =
      std::min<size_t>(bindings.size(), vertexBuffers.size() - slot);
  if (viewCount != bindings.size()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: ignoring %zu vertex buffer binding(s) not present in "
                 "the pipeline layout",
                 bindings.size() - viewCount);
  }

  Array<D3D12_VERTEX_BUFFER_VIEW> views(
      commandBuffer->getCreateInfo().allocator);
  views.resize(viewCount);
  for (size_t i = 0; i < viewCount; ++i) {
    auto buffer = downCast<Buffer>(bindings[i].buffer);
    if (!buffer) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "D3D12: null vertex buffer binding");
      return;
    }
    commandBuffer->keepAlive(buffer->getNative());
    commandBuffer->getDevice()->transition(
        commandBuffer->getNative(), buffer.get(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    views[i].BufferLocation = buffer->getDeviceAddress() + bindings[i].offset;
    views[i].SizeInBytes = buffer->getCreateInfo().size - bindings[i].offset;
    const auto &vb = vertexBuffers[slot + i];
    views[i].StrideInBytes = vb.pitch;
  }
  if (views.empty()) {
    return;
  }
  commandBuffer->getNative()->IASetVertexBuffers(
      slot, static_cast<UINT>(views.size()), views.data());
}

void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  auto buffer = downCast<Buffer>(binding.buffer);
  commandBuffer->keepAlive(buffer->getNative());
  commandBuffer->getDevice()->transition(commandBuffer->getNative(),
                                         buffer.get(),
                                         D3D12_RESOURCE_STATE_INDEX_BUFFER);
  D3D12_INDEX_BUFFER_VIEW view{};
  view.BufferLocation = buffer->getDeviceAddress() + binding.offset;
  view.SizeInBytes = buffer->getCreateInfo().size - binding.offset;
  view.Format = convert::indexFormatFrom(indexElementSize);
  commandBuffer->getNative()->IASetIndexBuffer(&view);
}

D3D12_GPU_DESCRIPTOR_HANDLE
CommandBuffer::samplerTable(const std::vector<Ptr<gpu::Sampler>> &samplers) {
  for (const auto &cached : samplerTables)
    if (cached.samplers == samplers)
      return cached.handle;
  D3D12_GPU_DESCRIPTOR_HANDLE first{};
  for (size_t i = 0; i < samplers.size(); ++i) {
    const auto descriptor = device->allocateTransientSamplerDescriptor();
    if (i == 0)
      first = descriptor.gpu;
    const auto source = samplers[i] ? downCast<Sampler>(samplers[i])->getCpu()
                                    : device->getDefaultSampler().cpu;
    device->getNative()->CopyDescriptorsSimple(
        1, descriptor.cpu, source, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  }
  // Retain source samplers so their CPU descriptor slots cannot be recycled
  // into different samplers while this command buffer's cache is in use.
  samplerTables.push_back({samplers, first});
  return first;
}

void RenderPass::bindFragmentSamplers(
    UInt32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  const UInt32 count =
      std::max<UInt32>(1, startSlot + static_cast<UInt32>(bindings.size()));
  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor firstSrv{};
  std::vector<Ptr<gpu::Sampler>> samplers(count);
  for (UInt32 i = 0; i < count; ++i) {
    auto srv = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      firstSrv = srv;
    }
    CpuGpuDescriptor srcSrv = device->getDefaultSrv();
    if (i >= startSlot && i < startSlot + bindings.size()) {
      const auto &binding = bindings[i - startSlot];
      if (binding.texture) {
        auto texture = downCast<Texture>(binding.texture);
        commandBuffer->keepAlive(texture->getNative());
        device->transition(commandBuffer->getNative(), texture.get(),
                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        srcSrv = {texture->getSrvCpu(), {}};
      }
      if (binding.sampler) {
        samplers[i] = binding.sampler;
      }
    }
    device->getNative()->CopyDescriptorsSimple(
        1, srv.cpu, srcSrv.cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  }
  const auto firstSampler = commandBuffer->samplerTable(samplers);
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(4, firstSrv.gpu);
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(5, firstSampler);
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(10, firstSrv.gpu);
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(11, firstSampler);
}

void RenderPass::bindFragmentSampler(UInt32 startSlot,
                                     const TextureSamplerBinding &binding) {
  Array<TextureSamplerBinding> bindings(
      commandBuffer->getCreateInfo().allocator);
  bindings.push_back(binding);
  bindFragmentSamplers(startSlot, bindings);
}

void RenderPass::bindAccelerationStructures(
    UInt32 startSlot,
    const Array<Ptr<gpu::AccelerationStructure>> &accelerationStructures) {
  if (!commandBuffer->getDevice()->supportsRayQuery()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: graphics acceleration structure binding ignored "
                "because ray query is disabled");
    return;
  }
  if (accelerationStructures.empty()) {
    return;
  }

  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor first{};
  const UInt32 descriptorCount =
      startSlot + static_cast<UInt32>(accelerationStructures.size());
  for (UInt32 i = 0; i < descriptorCount; ++i) {
    auto descriptor = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      first = descriptor;
    }
    if (i < startSlot) {
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
      desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      desc.ViewDimension =
          D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                    descriptor.cpu);
      continue;
    }
    auto accelerationStructure =
        downCast<AccelerationStructure>(accelerationStructures[i - startSlot]);
    if (!accelerationStructure) {
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
      desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      desc.ViewDimension =
          D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                    descriptor.cpu);
      continue;
    }
    commandBuffer->keepAlive(accelerationStructure->getNative());

    D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    desc.RaytracingAccelerationStructure.Location =
        accelerationStructure->getDeviceAddress();
    device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                  descriptor.cpu);
  }
  commandBuffer->getNative()->SetGraphicsRootDescriptorTable(12, first.gpu);
}

void RenderPass::setViewport(const Viewport &viewport) {
  D3D12_VIEWPORT vp{viewport.x,      viewport.y,        viewport.width,
                    viewport.height, viewport.minDepth, viewport.maxDepth};
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
    list->SetGraphicsRootConstantBufferView(6, address);
  }
  if (auto address = commandBuffer->vertexUniform(1)) {
    list->SetGraphicsRootConstantBufferView(1, address);
    list->SetGraphicsRootConstantBufferView(7, address);
  }
  if (auto address = commandBuffer->fragmentUniform(0)) {
    list->SetGraphicsRootConstantBufferView(2, address);
    list->SetGraphicsRootConstantBufferView(8, address);
  }
  if (auto address = commandBuffer->fragmentUniform(1)) {
    list->SetGraphicsRootConstantBufferView(3, address);
    list->SetGraphicsRootConstantBufferView(9, address);
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

ComputePass::ComputePass(CommandBuffer *commandBuffer,
                         const Array<StorageBufferBinding> &storageBuffers)
    : commandBuffer(commandBuffer),
      storageBuffers(storageBuffers, commandBuffer->getCreateInfo().allocator) {
}

void ComputePass::bindComputePipeline(
    Ptr<gpu::ComputePipeline> computePipeline) {
  pipeline = downCast<ComputePipeline>(computePipeline);
  if (!pipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: null compute pipeline binding");
    return;
  }
  commandBuffer->keepAlive(pipeline->getNative());
  commandBuffer->keepAlive(pipeline->getRootSignature());
  auto list = commandBuffer->getNative();
  list->SetComputeRootSignature(pipeline->getRootSignature());
  list->SetPipelineState(pipeline->getNative());
  bindStorageBuffers();
}

void ComputePass::bindAccelerationStructures(
    UInt32 startSlot,
    const Array<Ptr<gpu::AccelerationStructure>> &accelerationStructures) {
  if (!commandBuffer->getDevice()->supportsRayQuery()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "D3D12: compute acceleration structure binding ignored "
                "because ray query is disabled");
    return;
  }
  if (accelerationStructures.empty()) {
    return;
  }

  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor first{};
  const UInt32 descriptorCount =
      startSlot + static_cast<UInt32>(accelerationStructures.size());
  for (UInt32 i = 0; i < descriptorCount; ++i) {
    auto descriptor = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      first = descriptor;
    }
    if (i < startSlot) {
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
      desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      desc.ViewDimension =
          D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                    descriptor.cpu);
      continue;
    }
    auto accelerationStructure =
        downCast<AccelerationStructure>(accelerationStructures[i - startSlot]);
    if (!accelerationStructure) {
      D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
      desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      desc.ViewDimension =
          D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                    descriptor.cpu);
      continue;
    }
    commandBuffer->keepAlive(accelerationStructure->getNative());

    D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    desc.RaytracingAccelerationStructure.Location =
        accelerationStructure->getDeviceAddress();
    device->getNative()->CreateShaderResourceView(nullptr, &desc,
                                                  descriptor.cpu);
  }
  commandBuffer->getNative()->SetComputeRootDescriptorTable(5, first.gpu);
}

void ComputePass::bindStorageBuffers() {
  if (storageBuffers.empty()) {
    return;
  }

  auto device = commandBuffer->getDevice();
  CpuGpuDescriptor first{};
  for (UInt32 i = 0; i < storageBuffers.size(); ++i) {
    auto descriptor = device->allocateTransientSrvDescriptor();
    if (i == 0) {
      first = descriptor;
    }
    auto buffer = downCast<Buffer>(storageBuffers[i].buffer);
    if (!buffer) {
      continue;
    }
    commandBuffer->keepAlive(buffer->getNative());
    device->transition(commandBuffer->getNative(), buffer.get(),
                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.NumElements = buffer->getCreateInfo().size / 4;
    desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    device->getNative()->CreateUnorderedAccessView(buffer->getNative(), nullptr,
                                                   &desc, descriptor.cpu);
  }
  commandBuffer->getNative()->SetComputeRootDescriptorTable(0, first.gpu);
}

void ComputePass::bindUniforms() {
  auto list = commandBuffer->getNative();
  for (UInt32 i = 0; i < 4; ++i) {
    if (auto address = commandBuffer->computeUniform(i)) {
      list->SetComputeRootConstantBufferView(i + 1, address);
    }
  }
}

void ComputePass::dispatchWorkgroups(UInt32 groupCountX, UInt32 groupCountY,
                                     UInt32 groupCountZ) {
  if (!pipeline) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "D3D12: dispatch called without compute pipeline");
    return;
  }
  bindUniforms();
  commandBuffer->getNative()->Dispatch(groupCountX, groupCountY, groupCountZ);
}
} // namespace sinen::gpu::d3d12

#endif // SINEN_PLATFORM_WINDOWS
