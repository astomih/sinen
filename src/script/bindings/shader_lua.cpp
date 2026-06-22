#include "luaapi.hpp"
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <gpu/shader/shader.hpp>
#include <graphics/graphics.hpp>
#include <platform/io/asset_reader.hpp>
#include <shader_compiler/shader_compiler.hpp>

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

static int lShaderCompile(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  if (!AssetReader::exists(name)) {
    return luaLError2(L, "sn.Shader.compile asset not found: %s", name);
  }
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 2));
  auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 3));
  auto shader = makePtr<Shader>();
  shader->compile(StringView(name), stage, format);
  if (shader->getCode().size() == 0) {
    shader.reset();
    return luaLError2(L, "sn.Shader.compile failed: %s", name);
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

void registerShader(lua_State *L) {
  luaL_newmetatable(L, Shader::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lShaderGetCode);
  lua_setfield(L, -2, "getCode");
  luaPushcfunction2(L, lShaderIsReady);
  lua_setfield(L, -2, "isReady");
  luaPushcfunction2(L, lShaderGetNumSamplers);
  lua_setfield(L, -2, "getNumSamplers");
  luaPushcfunction2(L, lShaderGetNumStorageBuffers);
  lua_setfield(L, -2, "getNumStorageBuffers");
  luaPushcfunction2(L, lShaderGetNumStorageTextures);
  lua_setfield(L, -2, "getNumStorageTextures");
  luaPushcfunction2(L, lShaderGetNumUniformBuffers);
  lua_setfield(L, -2, "getNumUniformBuffers");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  luaPushcfunction2(L, lShaderNew);
  lua_setfield(L, -2, "new");
  luaPushcfunction2(L, lShaderCompile);
  lua_setfield(L, -2, "compile");
  luaPushcfunction2(L, lShaderCompileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  lua_pop(L, 1);
}
} // namespace sinen
