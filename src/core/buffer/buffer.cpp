#include "buffer.hpp"
#include <core/allocator/global_allocator.hpp>
#include <cstddef>
#include <graphics/camera/camera3d.hpp>
#include <math/matrix.hpp>
#include <math/vector.hpp>
#include <script/luaapi.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

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

static int lBufferFromBytes(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  const size_t n = lua_objlen(L, 1);
  Buffer buffer = makeBuffer(n, BufferType::Binary);
  auto *dst = static_cast<unsigned char *>(buffer.data());

  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));
    const auto value = static_cast<int>(luaL_checkinteger(L, -1));
    if (value < 0 || value > 255) {
      return luaLError2(L, "Buffer.fromBytes expects values in 0..255");
    }
    dst[i - 1] = static_cast<unsigned char>(value);
    lua_pop(L, 1);
  }

  udNewOwned<Buffer>(L, std::move(buffer));
  return 1;
}

static int lBufferToHeader(lua_State *L) {
  auto &buffer = udValue<Buffer>(L, 1);
  const char *symbolName = luaL_checkstring(L, 2);

  std::ostringstream out;
  out << "unsigned char " << symbolName << "[] = {\n";

  const auto *bytes = static_cast<const unsigned char *>(buffer.data());
  const size_t size = static_cast<size_t>(buffer.size());
  for (size_t i = 0; i < size; ++i) {
    if (i % 12 == 0) {
      out << "  ";
    }
    out << "0x" << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(bytes[i]);
    if (i + 1 < size) {
      out << ", ";
    }
    if (i % 12 == 11 || i + 1 == size) {
      out << "\n";
    }
  }

  out << std::dec << "};\n";
  out << "unsigned int " << symbolName << "_len = " << size << ";\n";

  const auto header = out.str();
  Buffer headerBuffer = makeBuffer(header.size(), BufferType::String);
  if (!header.empty()) {
    std::memcpy(headerBuffer.data(), header.data(), header.size());
  }
  udNewOwned<Buffer>(L, std::move(headerBuffer));
  return 1;
}

static int lBufferSize(lua_State *L) {
  auto &buffer = udValue<Buffer>(L, 1);
  lua_pushinteger(L, buffer.size());
  return 1;
}

void registerBuffer(lua_State *L) {
  luaL_newmetatable(L, Buffer::metaTableName());
  luaPushcfunction2(L, udGc<Buffer>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lBufferSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lBufferToHeader);
  lua_setfield(L, -2, "toHeader");
  lua_pop(L, 1);

  pushSnNamed(L, "Buffer");
  luaPushcfunction2(L, lBufferNew);
  lua_setfield(L, -2, "new");
  luaPushcfunction2(L, lBufferFromBytes);
  lua_setfield(L, -2, "fromBytes");
  lua_pop(L, 1);
}
} // namespace sinen
