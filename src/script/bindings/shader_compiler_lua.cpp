#include "luaapi.hpp"

#include "gpu/shader/shader_compiler_service.hpp"

#include <platform/io/asset_reader.hpp>

#include <string>
#include <utility>

namespace sinen {
namespace {

void pushBindings(lua_State *state,
                  const Array<CompiledShader::ResourceBinding> &bindings) {
  lua_createtable(state, static_cast<int>(bindings.size()), 0);
  for (size_t i = 0; i < bindings.size(); ++i) {
    lua_createtable(state, 0, 2);
    lua_pushlstring(state, bindings[i].name.data(), bindings[i].name.size());
    lua_setfield(state, -2, "name");
    lua_pushinteger(state, bindings[i].slot);
    lua_setfield(state, -2, "slot");
    lua_rawseti(state, -2, static_cast<lua_Integer>(i + 1));
  }
}

int pushCompiledShader(lua_State *state, CompiledShader compiled) {
  lua_createtable(state, 0, 7);
  lua_pushlstring(state, compiled.code.empty() ? "" : compiled.code.data(),
                  compiled.code.size());
  lua_setfield(state, -2, "code");
  lua_pushinteger(state, compiled.numCombinedSamplers);
  lua_setfield(state, -2, "numSamplers");
  lua_pushinteger(state, compiled.numStorageBuffers);
  lua_setfield(state, -2, "numStorageBuffers");
  lua_pushinteger(state, compiled.numStorageTextures);
  lua_setfield(state, -2, "numStorageTextures");
  lua_pushinteger(state, compiled.numUniformBuffers);
  lua_setfield(state, -2, "numUniformBuffers");
  pushBindings(state, compiled.uniformBuffers);
  lua_setfield(state, -2, "uniformBuffers");
  pushBindings(state, compiled.textures);
  lua_setfield(state, -2, "textures");
  return 1;
}

int compileSource(lua_State *state, StringView moduleName,
                  StringView modulePath, StringView source, ShaderStage stage,
                  ShaderFormat format) {
  CompiledShader compiled;
  std::string error;
  if (!compileShaderSource(moduleName, modulePath, source, stage, format,
                           compiled, error)) {
    return luaLError2(state, "ShaderCompiler.compile failed: %s",
                      error.c_str());
  }
  return pushCompiledShader(state, std::move(compiled));
}

int lShaderCompilerCompile(lua_State *state) {
  if (lua_gettop(state) != 3) {
    return luaLError2(
        state, "sn.ShaderCompiler.compile expects a path, stage, and format");
  }
  size_t pathSize = 0;
  const char *path = luaL_checklstring(state, 1, &pathSize);
  const StringView pathView(path, pathSize);
  if (!AssetReader::exists(pathView)) {
    return luaLError2(state, "sn.ShaderCompiler.compile asset not found: %s",
                      path);
  }
  const ShaderStage stage =
      static_cast<ShaderStage>(luaL_checkinteger(state, 2));
  const ShaderFormat format =
      static_cast<ShaderFormat>(luaL_checkinteger(state, 3));

  const size_t dot = pathView.find_last_of('.');
  const StringView moduleName =
      dot == StringView::npos ? pathView : pathView.substr(0, dot);
  const String source = AssetReader::readAsString(pathView);
  const String modulePath = AssetReader::getLoadPath(pathView);
  return compileSource(state, moduleName, modulePath, source, stage, format);
}

int lShaderCompilerCompileSource(lua_State *state) {
  if (lua_gettop(state) != 5) {
    return luaLError2(state,
                      "sn.ShaderCompiler.compileSource expects module name, "
                      "module path, source, stage, and format");
  }
  size_t moduleNameSize = 0;
  size_t modulePathSize = 0;
  size_t sourceSize = 0;
  const char *moduleName = luaL_checklstring(state, 1, &moduleNameSize);
  const char *modulePath = luaL_checklstring(state, 2, &modulePathSize);
  const char *source = luaL_checklstring(state, 3, &sourceSize);
  const ShaderStage stage =
      static_cast<ShaderStage>(luaL_checkinteger(state, 4));
  const ShaderFormat format =
      static_cast<ShaderFormat>(luaL_checkinteger(state, 5));
  return compileSource(state, StringView(moduleName, moduleNameSize),
                       StringView(modulePath, modulePathSize),
                       StringView(source, sourceSize), stage, format);
}

} // namespace

void registerShaderCompiler(lua_State *state) {
  pushSnNamed(state, "ShaderCompiler");
  Binding::registerFunction(state, "compile", lShaderCompilerCompile);
  Binding::registerFunction(state, "compileSource",
                            lShaderCompilerCompileSource);
  lua_pop(state, 1);
}

} // namespace sinen
