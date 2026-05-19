#include "compute_buffer.hpp"

#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <script/luaapi.hpp>

#include <cstring>

namespace sinen {
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

static int lComputeBufferNew(lua_State *L) {
  if (lua_gettop(L) == 0) {
    udPushPtr<ComputeBuffer>(L, makePtr<ComputeBuffer>());
    return 1;
  }
  if (lua_isnumber(L, 1)) {
    auto size = static_cast<UInt32>(luaL_checkinteger(L, 1));
    udPushPtr<ComputeBuffer>(L, makePtr<ComputeBuffer>(size));
    return 1;
  }
  auto &data = udValue<Buffer>(L, 1);
  udPushPtr<ComputeBuffer>(L, makePtr<ComputeBuffer>(data));
  return 1;
}

static int lComputeBufferCreate(lua_State *L) {
  auto &buffer = udPtr<ComputeBuffer>(L, 1);
  auto size = static_cast<UInt32>(luaL_checkinteger(L, 2));
  buffer->create(size);
  return 0;
}

static int lComputeBufferUpload(lua_State *L) {
  auto &buffer = udPtr<ComputeBuffer>(L, 1);
  auto &data = udValue<Buffer>(L, 2);
  buffer->upload(data);
  return 0;
}

static int lComputeBufferDownload(lua_State *L) {
  auto &buffer = udPtr<ComputeBuffer>(L, 1);
  udNewOwned<Buffer>(L, buffer->download());
  return 1;
}

static int lComputeBufferSize(lua_State *L) {
  auto &buffer = udPtr<ComputeBuffer>(L, 1);
  lua_pushinteger(L, buffer->size());
  return 1;
}

void registerComputeBuffer(lua_State *L) {
  luaL_newmetatable(L, ComputeBuffer::metaTableName());
  luaPushcfunction2(L, udPtrGc<ComputeBuffer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lComputeBufferCreate);
  lua_setfield(L, -2, "create");
  luaPushcfunction2(L, lComputeBufferUpload);
  lua_setfield(L, -2, "upload");
  luaPushcfunction2(L, lComputeBufferDownload);
  lua_setfield(L, -2, "download");
  luaPushcfunction2(L, lComputeBufferSize);
  lua_setfield(L, -2, "size");
  lua_pop(L, 1);

  pushSnNamed(L, "ComputeBuffer");
  luaPushcfunction2(L, lComputeBufferNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
