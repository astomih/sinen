#include <gpu/shader/shader_bundle.hpp>
#include <script/luaapi.hpp>


namespace sinen {
namespace {
uint32_t readOptionalU32Field(lua_State *L, int tableIndex, const char *name,
                              uint32_t defaultValue) {
  lua_getfield(L, tableIndex, name);
  uint32_t value = defaultValue;
  if (!lua_isnil(L, -1)) {
    value = static_cast<uint32_t>(luaL_checkinteger(L, -1));
  }
  lua_pop(L, 1);
  return value;
}
} // namespace

static int lShaderBundlePack(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  const size_t n = lua_objlen(L, 1);
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
    entry.data = buffer.data();
    entry.size = static_cast<uint32_t>(buffer.size());
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

  udNewOwned<Buffer>(L, ShaderBundle::packBuffer(entries));
  return 1;
}

void registerShaderBundle(lua_State *L) {
  pushSnNamed(L, "ShaderBundle");
  luaPushcfunction2(L, lShaderBundlePack);
  lua_setfield(L, -2, "pack");
  lua_pop(L, 1);
}

} // namespace sinen
