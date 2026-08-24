#include "luaapi.hpp"
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/shader/shader.hpp>
#include <graphics/graphics.hpp>
#include <platform/io/asset_reader.hpp>

namespace sinen {
static int lShaderNew(lua_State *L) {
  if (lua_gettop(L) != 2) {
    return luaLError2(L, "sn.Shader.new expects a path and shader stage");
  }
  const char *name = luaL_checkstring(L, 1);
  if (!AssetReader::exists(name)) {
    return luaLError2(L, "sn.Shader.new asset not found: %s", name);
  }
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 2));
  auto shader = makePtr<Shader>();
  shader->load(StringView(name), stage);
  udPushPtr<Shader>(L, std::move(shader));
  return 1;
}
static int lShaderCompileAndLoad(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  if (!AssetReader::exists(name)) {
    return luaLError2(L, "sn.Shader.compileAndLoad asset not found: %s", name);
  }
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 2));
  auto shader = makePtr<Shader>();

  if (lua_gettop(L) >= 3) {
    auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 3));
    shader->compileAndLoad(StringView(name), stage, format);
  } else {
    shader->compileAndLoad(StringView(name), stage);
  }
  udPushPtr<Shader>(L, std::move(shader));
  return 1;
}

static int lShaderGetCode(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  udNewOwned<Buffer>(L, s->getCode());
  return 1;
}

static int lShaderIsReady(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushboolean(L, s->isReady());
  return 1;
}

static int lShaderGetNumSamplers(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushinteger(L, s->getNumSamplers());
  return 1;
}

static int lShaderGetNumStorageBuffers(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushinteger(L, s->getNumStorageBuffers());
  return 1;
}

static int lShaderGetNumStorageTextures(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushinteger(L, s->getNumStorageTextures());
  return 1;
}

static int lShaderGetNumUniformBuffers(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  lua_pushinteger(L, s->getNumUniformBuffers());
  return 1;
}

static void
pushResourceBindings(lua_State *L,
                     const Array<Shader::ResourceBinding> &bindings) {
  lua_createtable(L, static_cast<int>(bindings.size()), 0);
  for (size_t i = 0; i < bindings.size(); ++i) {
    const auto &binding = bindings[i];
    lua_createtable(L, 0, 2);
    lua_pushlstring(L, binding.name.data(), binding.name.size());
    lua_setfield(L, -2, "name");
    lua_pushinteger(L, binding.slot);
    lua_setfield(L, -2, "slot");
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
}

static int lShaderGetUniformBufferBindings(lua_State *L) {
  pushResourceBindings(L, udPtr<Shader>(L, 1)->getUniformBufferBindings());
  return 1;
}

static int lShaderGetTextureBindings(lua_State *L) {
  pushResourceBindings(L, udPtr<Shader>(L, 1)->getTextureBindings());
  return 1;
}

void registerShader(lua_State *L) {
  luaL_newmetatable(L, Shader::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "getCode", lShaderGetCode);
  Binding::registerFunction(L, "isReady", lShaderIsReady);
  Binding::registerFunction(L, "getNumSamplers", lShaderGetNumSamplers);
  Binding::registerFunction(L, "getNumStorageBuffers",
                            lShaderGetNumStorageBuffers);
  Binding::registerFunction(L, "getNumStorageTextures",
                            lShaderGetNumStorageTextures);
  Binding::registerFunction(L, "getNumUniformBuffers",
                            lShaderGetNumUniformBuffers);
  Binding::registerFunction(L, "getUniformBufferBindings",
                            lShaderGetUniformBufferBindings);
  Binding::registerFunction(L, "getTextureBindings", lShaderGetTextureBindings);
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  Binding::registerFunction(L, "new", lShaderNew);
  Binding::registerFunction(L, "compileAndLoad", lShaderCompileAndLoad);
  lua_pop(L, 1);
}
} // namespace sinen
