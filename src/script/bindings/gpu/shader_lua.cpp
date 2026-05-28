#include <script/luaapi.hpp>
#include <gpu/shader.hpp>
#include <graphics/graphics.hpp>
#include <platform/io/asset_io.hpp>
#include <core/thread/future_poll.hpp>
#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <shader_compiler/shader_compiler.hpp>

namespace sinen {
static int lShaderNew(lua_State *L) {
  udPushPtr<Shader>(L, makePtr<Shader>());
  return 1;
}
static int lShaderLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  int numUniformData = static_cast<int>(luaL_checkinteger(L, 4));
  s->load(StringView(name), stage, numUniformData);
  return 0;
}
static int lShaderCompileAndLoad(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));

  if (lua_gettop(L) >= 4) {
    auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 4));
    s->compileAndLoad(StringView(name), stage, format);
  } else {
    s->compileAndLoad(StringView(name), stage);
  }
  return 0;
}

static int lShaderCompile(lua_State *L) {
  auto &s = udPtr<Shader>(L, 1);
  const char *name = luaL_checkstring(L, 2);
  ShaderStage stage = static_cast<ShaderStage>(luaL_checkinteger(L, 3));
  auto format = static_cast<ShaderFormat>(luaL_checkinteger(L, 4));
  s->compile(StringView(name), stage, format);
  return 0;
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

void registerShader(lua_State *L) {
  luaL_newmetatable(L, Shader::metaTableName());
  luaPushcfunction2(L, udPtrGc<Shader>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lShaderLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lShaderCompile);
  lua_setfield(L, -2, "compile");
  luaPushcfunction2(L, lShaderCompileAndLoad);
  lua_setfield(L, -2, "compileAndLoad");
  luaPushcfunction2(L, lShaderGetCode);
  lua_setfield(L, -2, "getCode");
  luaPushcfunction2(L, lShaderIsReady);
  lua_setfield(L, -2, "isReady");
  lua_pop(L, 1);

  pushSnNamed(L, "Shader");
  luaPushcfunction2(L, lShaderNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
