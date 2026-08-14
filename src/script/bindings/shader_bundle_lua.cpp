#include "luaapi.hpp"
#include <gpu/shader/shader_bundle.hpp>

#include <limits>

namespace sinen {
namespace {
uint32_t readOptionalU32Field(lua_State *L, int tableIndex, const char *name) {
  lua_getfield(L, tableIndex, name);
  uint32_t value = 0;
  if (!lua_isnil(L, -1)) {
    const auto raw = luaL_checkinteger(L, -1);
    if (raw < 0 ||
        static_cast<uint64_t>(raw) > std::numeric_limits<uint32_t>::max()) {
      luaLError2(L, "ShaderBundle field '%s' is out of uint32 range", name);
    }
    value = static_cast<uint32_t>(raw);
  }
  lua_pop(L, 1);
  return value;
}

void readResourceBindings(lua_State *L, int entryIndex, const char *field,
                          ShaderBundle::ResourceKind kind,
                          Array<ShaderBundle::ResourceBinding> &resources) {
  lua_getfield(L, entryIndex, field);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return;
  }
  luaL_checktype(L, -1, LUA_TTABLE);
  const int bindingsIndex = lua_gettop(L);
  const size_t count = lua_objlen(L, bindingsIndex);
  for (size_t i = 1; i <= count; ++i) {
    lua_rawgeti(L, bindingsIndex, static_cast<lua_Integer>(i));
    luaL_checktype(L, -1, LUA_TTABLE);
    const int bindingIndex = lua_gettop(L);

    lua_getfield(L, bindingIndex, "name");
    size_t nameSize = 0;
    const char *name = luaL_checklstring(L, -1, &nameSize);
    String bindingName(name, nameSize);
    lua_pop(L, 1);

    lua_getfield(L, bindingIndex, "slot");
    const auto rawSlot = luaL_checkinteger(L, -1);
    if (rawSlot < 0 ||
        static_cast<uint64_t>(rawSlot) > std::numeric_limits<uint32_t>::max()) {
      luaLError2(L, "ShaderBundle binding slot is out of uint32 range");
    }
    lua_pop(L, 1);
    resources.push_back(
        {kind, std::move(bindingName), static_cast<uint32_t>(rawSlot)});
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

Array<ShaderBundle::PackEntry> readPackEntries(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  const size_t count = lua_objlen(L, 1);
  if (count > std::numeric_limits<uint32_t>::max()) {
    luaLError2(L, "ShaderBundle entry count is out of range");
  }

  Array<ShaderBundle::PackEntry> entries;
  entries.reserve(count);
  for (size_t i = 1; i <= count; ++i) {
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
      luaLError2(L, "ShaderBundle entry %d code is too large",
                 static_cast<int>(i));
    }
    entry.data = buffer.data();
    entry.size = static_cast<uint32_t>(bufferSize);
    lua_pop(L, 1);

    entry.numSamplers = readOptionalU32Field(L, entryIndex, "numSamplers");
    entry.numStorageBuffers =
        readOptionalU32Field(L, entryIndex, "numStorageBuffers");
    entry.numStorageTextures =
        readOptionalU32Field(L, entryIndex, "numStorageTextures");
    entry.numUniformBuffers =
        readOptionalU32Field(L, entryIndex, "numUniformBuffers");
    readResourceBindings(L, entryIndex, "uniformBuffers",
                         ShaderBundle::ResourceKind::UniformBuffer,
                         entry.resources);
    readResourceBindings(L, entryIndex, "textures",
                         ShaderBundle::ResourceKind::Texture, entry.resources);

    entries.push_back(std::move(entry));
    lua_pop(L, 1);
  }
  return entries;
}
} // namespace

static int lShaderBundlePack(lua_State *L) {
  auto entries = readPackEntries(L);
  Buffer bundle = ShaderBundle::packBuffer(entries);
  if (bundle.size() == 0 && !entries.empty()) {
    return luaLError2(L, "ShaderBundle.pack failed to build bundle");
  }
  udNewOwned<Buffer>(L, std::move(bundle));
  return 1;
}

static int lShaderBundleDumpJson(lua_State *L) {
  auto entries = readPackEntries(L);
  String json = ShaderBundle::dumpJson(entries);
  if (json.empty() && !entries.empty()) {
    return luaLError2(L, "ShaderBundle.dumpJson failed to build JSON");
  }
  lua_pushlstring(L, json.data(), json.size());
  return 1;
}

void registerShaderBundle(lua_State *L) {
  pushSnNamed(L, "ShaderBundle");
  Binding::registerFunction(L, "pack", lShaderBundlePack);
  Binding::registerFunction(L, "dumpJson", lShaderBundleDumpJson);
  lua_pop(L, 1);
}
} // namespace sinen
