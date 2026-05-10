# [Luau API](https://github.com/astomih/sinen/blob/main/api/sinen.luau)

## 2D 座標

2D 描画は左上原点です。

- `(0, 0)` は `Camera2D` の左上
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
