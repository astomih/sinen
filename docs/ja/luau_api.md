# [Luau API](https://github.com/astomih/sinen/blob/main/api/sinen.luau)

## 2D 座標

2D 描画は左上原点です。

- `(0, 0)` はウィンドウの左上
- x は右方向に増加
- y は下方向に増加
- `Rect.new(x, y, width, height)` の `x, y` は矩形の左上
- `Graphics.drawRect` / `Graphics.drawImage` は `Rect` の左上を基準に描画
- `Graphics.drawText` の `position` は描画される文字列の左上

中心や右下など別の基準点から矩形を作りたい場合は `Pivot` 付きの `Rect.new` を使います。

```luau
local rect = sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(400, 300), sn.Vec2.new(120, 80))
sn.Graphics.drawRect(rect, sn.Color.new(1, 1, 1, 1))
```

2D 描画も明示的な `Camera2D` で scoped pass にできます。`begin2D` を使わない場合、または `finish()` 後は、ウィンドウが暗黙の 2D カメラとして使われます。

```luau
local uiCamera = sn.Camera2D.new()
uiCamera:resize(sn.Vec2.new(1280, 720))

sn.Graphics.begin2D(uiCamera)
sn.Graphics.drawText("HUD", font, sn.Vec2.new(20, 20))
sn.Graphics.finish()
```

## 3D Pass

3D 描画は明示的なカメラ pass で囲みます。`Graphics.begin3D(camera)` の後に 3D 描画を発行し、2D の暗黙描画へ戻る前に `Graphics.finish()` を呼びます。

```luau
local camera = sn.Camera3D.new()
camera:lookat(sn.Vec3.new(1, 1, 3), sn.Vec3.new(0), sn.Vec3.new(0, 1, 0))

sn.Graphics.begin3D(camera)
sn.Graphics.drawModel(model, transform)
sn.Graphics.finish()

sn.Graphics.drawText("HUD", font, sn.Vec2.new(20, 20))
```

## プロシージャル 3D モデル

`sn.MeshBuilder` は Luau から頂点・三角形・基本形状を組み立て、`Model` に変換できます。`vertex()` が返す頂点番号は Luau 側に合わせて 1 始まりです。

```luau
local builder = sn.MeshBuilder.new()
builder:addSphere(1.0, 16, 32, sn.Color.new(0.9, 0.4, 0.2, 1.0))

local model = builder:toModel()
local transform = sn.Transform.new()

sn.Graphics.begin3D(camera)
sn.Graphics.drawModel(model, transform)
sn.Graphics.finish()
```

## Immediate GUI

`sn.Gui` は小さな immediate-mode GUI レイヤーです。`draw()` から毎フレーム widget を呼ぶと、その場で描画され、新しい入力状態が戻り値で返ります。

```luau
local enabled = false
local volume = 0.5

function draw()
	if sn.Gui.button("Play", sn.Rect.new(20, 20, 120, 32)) then
		print("clicked")
	end

	enabled = sn.Gui.checkbox("Enabled", enabled, sn.Rect.new(20, 64, 160, 28))
	volume = sn.Gui.sliderFloat("Volume", volume, 0.0, 1.0, sn.Rect.new(20, 104, 240, 28))
end
```

## Video API

`sn.VideoWriter` と `sn.VideoReader` は、動画の書き出しと読み戻しに対応します。現在の実装は外部コーデックに依存しない簡易 AVI コンテナを使います。

```luau
local writer = sn.VideoWriter.new()
if writer:open("capture.avi", 640, 360, 30) then
	-- pixels は RGB8/RGBA8/BGR8/BGRA8 の sn.Buffer
	writer:addFrame(pixels, 640, 360, "rgba8", 90)
	writer:close()
end

local reader = sn.VideoReader.new()
if reader:open("capture.avi") then
	local info = reader:info()
	local frame = reader:readFrame(0)
	reader:close()
end
```
