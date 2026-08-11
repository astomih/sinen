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
  lua_getglobal(L, "sn");
  lua_newtable(L);
  lua_pushcfunction(L, hello, "Native.hello");
  lua_setfield(L, -2, "hello");
  lua_setfield(L, -2, "Native");
  lua_pop(L, 1);
  return 0;
}
```

Sinen calls `sinen_lua_module_open` as a protected Lua C function. Return `0` after successful registration; `luaL_error` can report a registration failure. A module may also export `sinen_lua_module_close(lua_State*)`, which Sinen calls before closing the VM.

C++ modules may use LuaBridge to register functions and classes. Sinen keeps each module loaded until the Luau VM has closed completely, so registered functions, classes, and `__gc` metamethods remain valid for the VM's lifetime.

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
local ok, err = sn.Script.loadNativeModule("plugins/my_native_module.dll")
if not ok then
  error(err)
end

print(sn.Native.hello())
```

For Emscripten, use a path such as `plugins/my_native_module.wasm`. The file must already exist in Emscripten's virtual filesystem, either through preloading or a JavaScript-side write.

C++ code can call `sinen::Script::loadNativeModule(path)` and retrieve failures through `sinen::Script::getNativeModuleError()`. Loading the same path again succeeds without opening a second copy.
