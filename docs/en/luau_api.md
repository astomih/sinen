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
