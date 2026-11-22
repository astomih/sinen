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

void registerComputeBuffer(lua_State *L) {
  pushClassMetatable<Ptr<ComputeBuffer>>(L, ComputeBuffer::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luau::table(L, -1).raw_function("create", lComputeBufferCreate);
  luau::table(L, -1).function("upload", lComputeBufferUpload);
  luau::table(L, -1).function("download", lComputeBufferDownload);
  luau::table(L, -1).function("size", lComputeBufferSize);
  lua_pop(L, 1);

  pushSnNamed(L, "ComputeBuffer");
  luau::table(L, -1).raw_function("new", lComputeBufferNew);
  lua_pop(L, 1);
}
} // namespace sinen
