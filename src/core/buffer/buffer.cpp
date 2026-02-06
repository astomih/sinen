#include "buffer.hpp"
#include <core/allocator/global_allocator.hpp>
#include <cstddef>
#include <graphics/camera/camera3d.hpp>
#include <math/matrix.hpp>
#include <math/vector.hpp>
#include <script/luaapi.hpp>

#include <cstring>

namespace sinen {
Buffer::Buffer(const BufferType &type, Ptr<void> data, size_t size)
    : _size(size), _type(type), _data(data) {}
int Buffer::size() const { return _size; }
BufferType Buffer::type() const { return _type; }
void *Buffer::data() const { return _data.get(); }
Buffer makeBuffer(size_t size, BufferType type, Allocator *allocator) {
  auto *ptr = allocator->allocate(size);
  auto deleter = Deleter<void>(allocator, size);
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}
Buffer makeBuffer(void *ptr, size_t size, BufferType type,
                  Allocator *allocator) {
  auto deleter = Deleter<void>(allocator, size);
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}

static int lBufferNew(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  Array<void *> chunks;
  Array<size_t> chunkSizes;

  size_t n = lua_objlen(L, 1);
  chunks.reserve(n);
  chunkSizes.reserve(n);

  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));

    if (auto *v3 = udValueOrNull<Vec3>(L, -1)) {
      size_t s = sizeof(Vec3);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v3, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *v2 = udValueOrNull<Vec2>(L, -1)) {
      size_t s = sizeof(Vec2);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, v2, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (auto *cam = udValueOrNull<Camera3D>(L, -1)) {
      size_t s = sizeof(Mat4) * 2;
      void *p = GlobalAllocator::get()->allocate(s);
      auto view = cam->getView();
      auto proj = cam->getProjection();
      std::memcpy(p, &view, sizeof(Mat4));
      std::memcpy(reinterpret_cast<std::byte *>(p) + sizeof(Mat4), &proj,
                  sizeof(Mat4));
      chunks.push_back(p);
      chunkSizes.push_back(s);
    } else if (lua_isnumber(L, -1)) {
      float v = static_cast<float>(lua_tonumber(L, -1));
      size_t s = sizeof(float);
      void *p = GlobalAllocator::get()->allocate(s);
      std::memcpy(p, &v, s);
      chunks.push_back(p);
      chunkSizes.push_back(s);
    }

    lua_pop(L, 1);
  }

  size_t total = 0;
  for (auto s : chunkSizes) {
    total += s;
  }
  void *ptr = GlobalAllocator::get()->allocate(total);
  size_t offset = 0;
  for (size_t i = 0; i < chunks.size(); ++i) {
    std::memcpy(reinterpret_cast<std::byte *>(ptr) + offset, chunks[i],
                chunkSizes[i]);
    GlobalAllocator::get()->deallocate(chunks[i], chunkSizes[i]);
    offset += chunkSizes[i];
  }

  Buffer buffer(BufferType::Binary,
                Ptr<void>(ptr, Deleter<void>(GlobalAllocator::get(), total)),
                total);
  udNewOwned<Buffer>(L, std::move(buffer));
  return 1;
}
void registerBuffer(lua_State *L) {
  luaL_newmetatable(L, Buffer::metaTableName());
  luaPushcfunction2(L, udGc<Buffer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  pushSnNamed(L, "Buffer");
  luaPushcfunction2(L, lBufferNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
