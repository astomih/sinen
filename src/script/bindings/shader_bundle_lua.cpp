#include "luaapi.hpp"
#include <gpu/shader/shader_bundle.hpp>


#include <limits>

namespace sinen {
namespace {
uint32_t readOptionalU32Field(lua_State *L, int tableIndex, const char *name,
                              uint32_t defaultValue) {
  lua_getfield(L, tableIndex, name);
  uint32_t value = defaultValue;
  if (!lua_isnil(L, -1)) {
    const auto raw = luaL_checkinteger(L, -1);
    if (raw < 0 ||
        static_cast<uint64_t>(raw) > std::numeric_limits<uint32_t>::max()) {
      luaLError2(L, "ShaderBundle.pack field '%s' is out of uint32 range",
                 name);
    }
    value = static_cast<uint32_t>(raw);
  }
  lua_pop(L, 1);
  return value;
}
} // namespace

static int lShaderBundlePack(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  const size_t n = lua_objlen(L, 1);
  if (n > std::numeric_limits<uint32_t>::max()) {
    return luaLError2(L, "ShaderBundle.pack entry count is out of range");
  }

  Array<ShaderBundle::PackEntry> entries;
  entries.reserve(n);

  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 1, static_cast<lua_Integer>(i));
    luaL_checktype(L, -1, LUA_TTABLE);
    const int entryIndex = lua_gettop(L);

    ShaderBundle::PackEntry entry;
    lua_getfield(L, entryIndex, "stage");
    entry.stage = static_cast<ShaderStage>(luaL_checkinteger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, entryIndex, "format");
    entry.format = static_cast<ShaderFormat>(luaL_checkinteger(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, entryIndex, "code");
    auto &buffer = udValue<Buffer>(L, -1);
    const int bufferSize = buffer.size();
    if (bufferSize < 0) {
      return luaLError2(L, "ShaderBundle.pack entry %d code is too large",
                        static_cast<int>(i));
    }
    entry.data = buffer.data();
    entry.size = static_cast<uint32_t>(bufferSize);
    lua_pop(L, 1);

    entry.numSamplers = readOptionalU32Field(L, entryIndex, "numSamplers", 0);
    entry.numStorageBuffers =
        readOptionalU32Field(L, entryIndex, "numStorageBuffers", 0);
    entry.numStorageTextures =
        readOptionalU32Field(L, entryIndex, "numStorageTextures", 0);
    entry.numUniformBuffers =
        readOptionalU32Field(L, entryIndex, "numUniformBuffers", 0);

    entries.push_back(entry);
    lua_pop(L, 1);
  }

  Buffer bundle = ShaderBundle::packBuffer(entries);
  if (bundle.size() == 0 && n > 0) {
    return luaLError2(L, "ShaderBundle.pack failed to build bundle");
  }
  udNewOwned<Buffer>(L, std::move(bundle));
  return 1;
}

void registerShaderBundle(lua_State *L) {
  pushSnNamed(L, "ShaderBundle");
  luaPushcfunction2(L, lShaderBundlePack);
  lua_setfield(L, -2, "pack");
  lua_pop(L, 1);
}

} // namespace sinen
