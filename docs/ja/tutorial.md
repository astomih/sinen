# Sinenチュートリアル

このページでは、Sinen を使って Luau でアプリを動かす最短手順を説明します。

## 1. まず覚えること

Sinen の基本はこの3つです。

- `main.luau` をエントリースクリプトとして実行する
- `setup` / `update` / `draw` の3関数で処理を分ける
- アセットは実行時のベースディレクトリ配下に置く

最小コードは次のとおりです。

```luau
local sn = require("@sinen")
local font: sn.Font = sn.Font.new()

function setup()
	font:load(32)
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		-- 必要なら別シーンへ遷移
		-- sn.Script.load("main", ".")
	end
end

function draw()
	sn.Graphics.drawText("Hello Sinen!", font, sn.Vec2.new(0, 0), sn.Color.new(1.0), 32)
end
```

## 2. 実行方法

### Windows (このリポジトリの構成)

1. ルートでビルド

```bat
build\generate_msvc2026-debug.bat
build\build_msvc2026-debug.bat
```

2. プロジェクトディレクトリへ移動して実行

```bat
cd examples\01_helloworld
..\..\build\msvc2026-debug\sinen.exe
```

`main.luau` が読み込まれます。

### Android

- `build\generate_android.bat`
- `build\build_android.bat`
- `build/android` を Android Studio で開いて実行
- アセット一式を `/sdcard/Android/media/astomih.sinen.app` に配置

## 3. ライフサイクル

- `setup()`: シーン読込時に1回。テクスチャやモデルのロードに使う
- `update()`: 毎フレーム更新。入力・ゲームロジック・物理更新に使う
- `draw()`: 毎フレーム描画

`setup` は省略可能です。`update` / `draw` だけでも動作します。

## 4. 画像を描く

```luau
local sn = require("@sinen")
local image: sn.Texture = sn.Texture.new()

function setup()
	image:load("logo.png")
end

function draw()
	sn.Graphics.drawImage(image, sn.Rect.new(sn.Vec2.new(0), image:size()))
end
```

## 5. 入力とサウンド

```luau
local sn = require("@sinen")
local se: sn.Sound = sn.Sound.new()

function setup()
	se:load("shot.wav")
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
		se:play()
	end
end
```

## 6. シーン遷移

`sn.Script.load(filePath, baseDirPath)` で遷移できます。

- `filePath`: 拡張子なしのスクリプト名（例: `main`）
- `baseDirPath`: そのシーンの基準ディレクトリ

例: `scenes/title/main.luau` に遷移

```luau
sn.Script.load("main", "scenes/title")
```

## 7. ディレクトリ構成の例

```text
my_game/
  main.luau
  player.luau
  logo.png
  shot.wav
  scenes/
    title/
      main.luau
```

同一ディレクトリのモジュールは通常の `require` で読み込めます。

```luau
local Player = require("./player")
```

## 8. 次に読むサンプル

- `examples/01_helloworld`: 最小構成
- `examples/02_texture`: 画像表示
- `examples/03_model`: 3Dモデル表示
- `examples/04_sound`: サウンド再生
- `examples/09_physics`: 物理
- `examples/14_synth`: シンセ

API 一覧は `docs/ja/luau_api.md` から参照できます。
