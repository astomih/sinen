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

static auto lShaderGetCode(Ptr<Shader> &s) { return s->getCode(); }

static auto lShaderIsReady(Ptr<Shader> &s) { return s->isReady(); }

static auto lShaderGetNumSamplers(Ptr<Shader> &s) {
  return s->getNumSamplers();
}

static auto lShaderGetNumStorageBuffers(Ptr<Shader> &s) {
  return s->getNumStorageBuffers();
}

static auto lShaderGetNumStorageTextures(Ptr<Shader> &s) {
  return s->getNumStorageTextures();
}

static auto lShaderGetNumUniformBuffers(Ptr<Shader> &s) {
  return s->getNumUniformBuffers();
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

void registerShader(luau::table &sn) {
  auto shader = engineClass<Ptr<Shader>>(sn, Shader::metaTableName());

  shader.method<lShaderGetCode>("getCode");
  shader.method<lShaderIsReady>("isReady");
  shader.method<lShaderGetNumSamplers>("getNumSamplers");
  shader.method<lShaderGetNumStorageBuffers>("getNumStorageBuffers");
  shader.method<lShaderGetNumStorageTextures>("getNumStorageTextures");
  shader.method<lShaderGetNumUniformBuffers>("getNumUniformBuffers");
  shader.raw_method<lShaderGetUniformBufferBindings>(
      "getUniformBufferBindings");
  shader.raw_method<lShaderGetTextureBindings>("getTextureBindings");

  shader.raw_static_function<lShaderNew>("new");
  shader.raw_static_function<lShaderCompileAndLoad>("compileAndLoad");
}
} // namespace sinen
