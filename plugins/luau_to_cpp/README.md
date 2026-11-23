# Luau → C++

Luau の構文木から、数値計算の関数を C++ ヘッダーに変換します。Sinen のネイティブプラグインとビルド用の CLI が同じ変換処理を使います。

Windows の Debug ビルドでは、`build\build_msvc2026-debug.bat` が `luau_to_cpp.dll` と `sinen_luau_to_cpp.exe` を `build\msvc2026-debug` に出力します。Sinen は作業ディレクトリからプラグインを読み込むので、そのディレクトリで起動するか、DLL を起動時の作業ディレクトリに配置してください。

Sinen の Luau スクリプトでファイルを変換する例です。`input.luau` はスクリプトから読み込める場所に置きます。出力は Sinen のユーザーディレクトリに保存します。

```luau
local sn = require("@sinen")
local loaded, loadError = sn.Script.loadPlugin("luau_to_cpp")
assert(loaded, loadError)
local converter = require("@sinen/plugin/luau_to_cpp")

local input = "input.luau"
local source = assert(sn.Filesystem.readText(input), "入力を読み込めません")
local cpp, diagnostic = converter.convert(source, input)
assert(cpp, diagnostic)
assert(sn.Filesystem.writeText("user://converted.hpp", cpp), "出力を書き込めません")
```

`convert(source, sourceName?)` は、成功時に `cpp, nil`、失敗時に `nil, diagnostic` を返します。診断には入力名と行・列が含まれます。

CLI を使う場合は、リポジトリのルートで以下を実行します。

```powershell
build\msvc2026-debug\sinen_luau_to_cpp.exe game\fps\slab.luau build\msvc2026-debug\slab.hpp
```

入力は、引数と戻り値に `number` を指定したローカル関数を一つ定義し、その関数を返す Luau モジュールです。数値のローカル変数、算術、比較、論理演算、条件分岐、複数の数値の返却、`math.abs`、`math.sqrt`、引数二つの `math.min`・`math.max`、`math.huge` に対応しています。生成したヘッダーでは `sinen::generated::run` として呼び出せます。
