# Sinenチュートリアル

このページでは、Sinen を使って Luau でアプリを動かす最短手順を説明します。

## 1. まず覚えること

Sinen の基本はこの3つです。

- `main.snb` または `main.luau` をエントリースクリプトとして実行する
- `setup` / `update` / `draw` の3関数で処理を分ける
- アセットは実行時のベースディレクトリ配下に置く

2D 描画の座標は Processing や raylib と同じく左上原点です。`sn.Vec2.new(0, 0)` は画面左上、x は右方向、y は下方向に増えます。`sn.Rect.new(x, y, width, height)` の `x, y` も矩形の左上です。

最小コードは次のとおりです。

```luau
local sn = require("@sinen")
local font: sn.Font = sn.Font.new(32)

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		sn.TextStyle.new(font, sn.Color.new(1.0), 32),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
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
cd examples\basics\01_helloworld
..\..\..\build\msvc2026-debug\sinen.exe
```

`main.snb` があればそれが読み込まれ、なければ `main.luau` が読み込まれます。

`.sna` アーカイブを渡して実行することもできます。`.sna` は zip 形式で、
zip 直下に `sna/` フォルダを置き、その中に `main.luau` とアセットを配置します。
`main.snb` がある場合は `main.luau` より優先されます。

```text
hoge.sna
└─ sna
   ├─ main.luau
   └─ logo.png
```

```bat
build\msvc2026-debug\sinen.exe hoge.sna
```

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
local image: sn.Texture = sn.Texture.new("logo.png")

function setup()
end

function draw()
	sn.Graphics.drawImage(image, sn.Rect.new(20, 20, image:size().x, image:size().y))
end
```

## 5. 入力とサウンド

```luau
local sn = require("@sinen")
local se: sn.Sound = sn.Sound.new("shot.wav")

function setup()
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then
		se:play()
	end
end
```

## 6. シーン遷移

`sn.Script.load(path)` で遷移できます。

- `path`: `.luau` または `.snb` 付きのスクリプトパス（例: `scenes/title/main.luau`）
- 相対パスは現在のシーンファイルの場所基準、`/` 始まりはルートシーンファイルの場所基準です

例: `scenes/title/main.luau` に遷移

```luau
sn.Script.load("/scenes/title/main.luau")
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
API 一覧は `docs/ja/luau_api.md` から参照できます。
