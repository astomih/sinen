#include "luaapi.hpp"
#include <core/allocator/global_allocator.hpp>
#include <gpu/compute/compute_buffer.hpp>
#include <graphics/graphics.hpp>

namespace sinen {
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

static auto lComputeBufferUpload(Ptr<ComputeBuffer> &buffer, Buffer &data) {
  buffer->upload(data);
  return;
}

static auto lComputeBufferDownload(Ptr<ComputeBuffer> &buffer) {
  return buffer->download();
}

static auto lComputeBufferSize(Ptr<ComputeBuffer> &buffer) {
  return buffer->size();
}

void registerComputeBuffer(luau::table &sn) {
  auto computeBuffer =
      engineClass<Ptr<ComputeBuffer>>(sn, ComputeBuffer::metaTableName());

  computeBuffer.raw_method<lComputeBufferCreate>("create");
  computeBuffer.method<lComputeBufferUpload>("upload");
  computeBuffer.method<lComputeBufferDownload>("download");
  computeBuffer.method<lComputeBufferSize>("size");

  computeBuffer.raw_static_function<lComputeBufferNew>("new");
}
} // namespace sinen
