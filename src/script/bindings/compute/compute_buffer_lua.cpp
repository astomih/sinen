#include <script/luaapi.hpp>
#include <compute/compute_buffer.hpp>
#include <core/allocator/global_allocator.hpp>
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
