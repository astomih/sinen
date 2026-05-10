# [Luau API](https://github.com/astomih/sinen/blob/main/api/sinen.luau)

## 2D Coordinates

2D drawing uses a top-left origin.

- `(0, 0)` is the top-left of the active `Camera2D`
- x grows to the right
- y grows downward
- `Rect.new(x, y, width, height)` treats `x, y` as the rectangle's top-left
- `Graphics.drawRect` and `Graphics.drawImage` draw from the `Rect` top-left
- `Graphics.drawText` uses the top-left of the rendered text as `position`

Use the `Pivot` overload of `Rect.new` when you want to construct a rectangle from another anchor such as the center or bottom-right.

```luau
local rect = sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(400, 300), sn.Vec2.new(120, 80))
sn.Graphics.drawRect(rect, sn.Color.new(1, 1, 1, 1))
```

## Video API

`sn.VideoWriter` and `sn.VideoReader` write and read video files. The current implementation uses a simple AVI container without an external codec dependency.

```luau
local writer = sn.VideoWriter.new()
if writer:open("capture.avi", 640, 360, 30) then
	-- pixels is an RGB8/RGBA8/BGR8/BGRA8 sn.Buffer
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
