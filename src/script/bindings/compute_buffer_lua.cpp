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
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "create", lComputeBufferCreate);
  Binding::registerFunction(L, "upload", lComputeBufferUpload);
  Binding::registerFunction(L, "download", lComputeBufferDownload);
  Binding::registerFunction(L, "size", lComputeBufferSize);
  lua_pop(L, 1);

  pushSnNamed(L, "ComputeBuffer");
  Binding::registerFunction(L, "new", lComputeBufferNew);
  lua_pop(L, 1);
}
} // namespace sinen
