# Native Lua modules

Sinen can load a native module into the running Luau VM and pass its registration function the VM's `lua_State*`. A module is a DLL on Windows and a dynamic-linking Wasm side module under Emscripten.

Native modules execute arbitrary code in the process. Load only trusted modules.

## C API

Include `script/sinen_lua_module.h` and Luau's `lua.h`, then export the API version and registration function through the C ABI. Luau's own API symbols use C++ linkage, so compile the module source as C++.

```cpp
#include <lua.h>
#include <script/sinen_lua_module.h>

static int hello(lua_State *L) {
  lua_pushstring(L, "hello from native code");
  return 1;
}

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *L) {
  lua_newtable(L);
  lua_pushcfunction(L, hello, "my_native_module.hello");
  lua_setfield(L, -2, "hello");
  lua_setfield(L, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "my_native_module");
  return 0;
}
```

Sinen calls `sinen_lua_module_open` as a protected Lua C function. Return `0` after successful registration; `luaL_error` can report a registration failure. A module may also export `sinen_lua_module_close(lua_State*)`, which Sinen calls before closing the VM.

Register functions and classes with the Luau C API. Sinen keeps each module loaded until the Luau VM has closed completely, so registered C functions and `__gc` metamethods remain valid for the VM's lifetime.

## Building

When the module is part of the same CMake build as Sinen, use the supplied helper:

```cmake
sinen_add_lua_module(my_native_module my_native_module.cpp)
```

On Windows this produces `my_native_module.dll` linked against the same Luau revision. Under Emscripten it produces `my_native_module.wasm` with `-sSIDE_MODULE=1`. The Sinen application uses `-sMAIN_MODULE=2` and retains the Lua C API exports so the side module can resolve its imports.

Build the module and Sinen against the same Luau headers and library revision, using compatible compiler settings.

## Loading from Luau

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("my_native_module")
if not ok then
  error(err)
end

local nativeModule = require("@sinen/plugin/my_native_module")
print(nativeModule.hello())
```

When omitted, the platform extension (`.dll` on Windows, `.wasm` on Emscripten, and so on) is appended automatically. Plugin names cannot contain directory separators or absolute paths; plugins are loaded only from the process current directory. Register a plugin's module table under a registry key formed from `SINEN_LUA_PLUGIN_REGISTRY_PREFIX` and the same plugin name. After loading, scripts can retrieve it with `require("@sinen/plugin/<plugin name>")`. A Wasm file must already exist in Emscripten's virtual filesystem, either through preloading or a JavaScript-side write.

C++ code can call `sinen::Script::load_plugin(name)` and retrieve failures through `sinen::Script::getPluginError()`. Loading the same plugin again succeeds without opening a second copy. The former `loadNativeModule` API remains as a compatibility alias and applies the same current-directory restriction.

## nativefs

Configure with `SINEN_PLUGIN_NATIVEFS=ON` to build the `nativefs` plugin, which can read absolute paths outside the normal filesystem sandbox. The option is `OFF` by default. Place the resulting plugin in Sinen's current directory and load it to access `read`, `readText`, `exists`, `enumerateDirectory`, `getCurrentDirectory`, and `getAbsolutePath` through `require("@sinen/plugin/nativefs")`.

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("nativefs")
if not ok then
  error(err)
end

local nativefs = require("@sinen/plugin/nativefs")
local text, readError = nativefs.readText("C:/data/example.txt")
if not text then
  error(readError)
end
```

`nativefs` deliberately bypasses the normal `sn.Filesystem` sandbox. Load it only from trusted scripts.

## shader_compiler

Configure with `SINEN_PLUGIN_SHADER_COMPILER=ON` to build the `shader_compiler` plugin containing Slang and the ShaderCompiler implementation. The option defaults to `ON` for native builds and is forced `OFF` under Emscripten. Slang is not linked into the Sinen application.

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("shader_compiler")
if not ok then
  error(err)
end

local ShaderCompiler = require("@sinen/plugin/shader_compiler")
local compiled = ShaderCompiler.compile(
  "shader.slang",
  sn.ShaderStage.Fragment,
  sn.ShaderFormat.SPIRV
)
```

The former `sn.Shader.compile` function has been removed; use `ShaderCompiler.compile` from `require("@sinen/plugin/shader_compiler")` for compilation without loading a GPU shader. Its `code` result is a binary string that can be passed directly to a `sn.ShaderBundle.pack` entry. `sn.Shader.compileAndLoad` also uses this plugin-backed compiler service, so load the plugin before calling it.
