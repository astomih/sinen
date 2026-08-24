# ネイティブ Lua モジュール

Sinen は実行中の Luau VM にネイティブモジュールを読み込み、モジュールの登録関数へ `lua_State*` を渡せます。Windows では DLL、Emscripten では dynamic linking の side module としてビルドした Wasm を使用します。

ネイティブモジュールはプロセス内で任意のコードを実行できます。信頼できるモジュールだけをロードしてください。

## C API

モジュールは `script/sinen_lua_module.h` と Luau の `lua.h` をインクルードし、C ABI で API バージョンと登録関数をエクスポートします。Luau の API シンボル自体は C++ linkage なので、ソースは C++ としてコンパイルしてください。

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

`sinen_lua_module_open` は保護された Lua C 関数として呼ばれます。登録が成功した場合は `0` を返してください。登録に失敗した場合は `luaL_error` を使用できます。任意の `sinen_lua_module_close(lua_State*)` をエクスポートすると、VM の終了前に呼ばれます。

関数やクラスは Luau C API を使って登録してください。モジュールはロード後から Luau VM が完全に閉じるまでアンロードされないため、登録した C 関数と `__gc` メタメソッドはその期間中有効です。

## ビルド

Sinen と同じ CMake ビルド内では、用意されたヘルパーを使えます。

```cmake
sinen_add_lua_module(my_native_module my_native_module.cpp)
```

Windows では `my_native_module.dll` が生成され、同じ Luau リビジョンへリンクされます。Emscripten では `-sSIDE_MODULE=1` を指定した `my_native_module.wasm` が生成されます。Sinen 本体には `-sMAIN_MODULE=2` が設定され、保持された Lua C API export から side module の import を解決します。

モジュールと Sinen 本体は、同じソースツリーの Luau ヘッダー・ライブラリ、および互換性のあるコンパイラ設定でビルドしてください。

## Luau からロード

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("my_native_module")
if not ok then
  error(err)
end

print(sn.Native.hello())
```

拡張子を省略すると、Windows では `.dll`、Emscripten では `.wasm` など、プラットフォームの拡張子が自動で補われます。プラグイン名にはディレクトリ区切りや絶対パスを指定できず、プロセスのカレントディレクトリ直下からのみロードされます。Wasm ファイルは、事前ロードまたは JavaScript からの書き込みによって Emscripten の仮想ファイルシステム上に配置してからロードしてください。

C++ からは `sinen::Script::load_plugin(name)` を使用できます。失敗理由は `sinen::Script::getPluginError()` で取得できます。同じプラグインを再度指定した場合は既にロード済みとして成功します。従来の `loadNativeModule` は互換エイリアスとして残っていますが、同じカレントディレクトリ制約が適用されます。

## nativefs

`SINEN_PLUGIN_NATIVEFS=ON` で構成すると、サンドボックス外の絶対パスを読み取れる `nativefs` プラグインをビルドします。このオプションはデフォルトで `OFF` です。生成されたプラグインを Sinen と同じカレントディレクトリに置いてロードすると、グローバルテーブル `nativefs` に `read`、`readText`、`exists`、`enumerateDirectory`、`getCurrentDirectory`、`getAbsolutePath` が登録されます。

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("nativefs")
if not ok then
  error(err)
end

local text, readError = nativefs.readText("C:/data/example.txt")
if not text then
  error(readError)
end
```

`nativefs` は意図的に通常の `sn.Filesystem` のサンドボックスを迂回します。信頼できるスクリプトからだけロードしてください。

## shader_compiler

`SINEN_PLUGIN_SHADER_COMPILER=ON` で構成すると、Slang と ShaderCompiler 実装を含む `shader_compiler` プラグインをビルドします。ネイティブビルドではデフォルトで `ON` ですが、Emscriptenでは強制的に `OFF` になります。Slang はSinen本体にはリンクされません。

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("shader_compiler")
if not ok then
  error(err)
end

local compiled = sn.ShaderCompiler.compile(
  "shader.slang",
  sn.ShaderStage.Fragment,
  sn.ShaderFormat.SPIRV
)
```

従来の `sn.Shader.compile` は廃止されています。コンパイルのみを行う場合は `sn.ShaderCompiler.compile` を使用してください。返される `code` はバイナリ文字列で、そのまま `sn.ShaderBundle.pack` のエントリに渡せます。`sn.Shader.compileAndLoad` もこのプラグインのコンパイラサービスを使用するため、呼び出す前にプラグインをロードする必要があります。
