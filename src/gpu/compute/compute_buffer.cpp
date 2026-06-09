#include "compute_buffer.hpp"

#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>

#include <cstring>

namespace sinen {
ComputeBuffer::~ComputeBuffer() {
  buffer.reset();
  bufferSize = 0;
}
ComputeBuffer::ComputeBuffer(UInt32 size) { create(size); }

ComputeBuffer::ComputeBuffer(const Buffer &data) { upload(data); }

void ComputeBuffer::create(UInt32 size) {
  bufferSize = size;
  gpu::Buffer::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.usage = gpu::BufferUsage::Storage;
  info.size = size;
  buffer = Graphics::getDevice()->createBuffer(info);
}

void ComputeBuffer::upload(const Buffer &data) {
  if (data.size() == 0) {
    return;
  }
  if (!buffer || bufferSize != static_cast<UInt32>(data.size())) {
    create(static_cast<UInt32>(data.size()));
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  gpu::TransferBuffer::CreateInfo transferInfo{};
  transferInfo.allocator = allocator;
  transferInfo.size = static_cast<UInt32>(data.size());
  transferInfo.usage = gpu::TransferBufferUsage::Upload;
  auto transfer = device->createTransferBuffer(transferInfo);
  auto *mapped = transfer->map(true);
  if (mapped) {
    std::memcpy(mapped, data.data(), data.size());
  }
  transfer->unmap();

  auto commandBuffer = device->acquireCommandBuffer({allocator});
  auto copyPass = commandBuffer->beginCopyPass();
  gpu::BufferTransferInfo src{};
  src.transferBuffer = transfer;
  src.offset = 0;
  gpu::BufferRegion dst{};
  dst.buffer = buffer;
  dst.offset = 0;
  dst.size = bufferSize;
  copyPass->uploadBuffer(src, dst, true);
  commandBuffer->endCopyPass(copyPass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();
}

Buffer ComputeBuffer::download() const {
  if (!buffer || bufferSize == 0) {
    return Buffer(BufferType::Binary, Ptr<void>(), 0);
  }

  auto allocator = GlobalAllocator::get();
  auto device = Graphics::getDevice();
  gpu::TransferBuffer::CreateInfo transferInfo{};
  transferInfo.allocator = allocator;
  transferInfo.size = bufferSize;
  transferInfo.usage = gpu::TransferBufferUsage::Download;
  auto transfer = device->createTransferBuffer(transferInfo);

  auto commandBuffer = device->acquireCommandBuffer({allocator});
  auto copyPass = commandBuffer->beginCopyPass();
  gpu::BufferRegion src{};
  src.buffer = buffer;
  src.offset = 0;
  src.size = bufferSize;
  gpu::BufferTransferInfo dst{};
  dst.transferBuffer = transfer;
  dst.offset = 0;
  copyPass->downloadBuffer(src, dst);
  commandBuffer->endCopyPass(copyPass);
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();

  Buffer out = makeBuffer(bufferSize, BufferType::Binary);
  auto *mapped = transfer->map(false);
  if (mapped) {
    std::memcpy(out.data(), mapped, bufferSize);
  }
  transfer->unmap();
  return out;
}

} // namespace sinen
