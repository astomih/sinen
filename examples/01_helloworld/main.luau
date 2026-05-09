local sn = require("@sinen")
local font: sn.Font = sn.Font.new()
font:load(32)

local function textTopLeftFromCenter(text: string, pos: sn.Vec2, fontSize: number): sn.Vec2
	local half = sn.Graphics.getCamera2D():half()
	local screenPos = sn.Vec2.new(half.x + pos.x, half.y - pos.y)
	return font:region(text, fontSize, sn.Pivot.Center, screenPos):topLeft()
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
end

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		font,
		textTopLeftFromCenter("Hello World!", sn.Vec2.new(0, 0), 32),
		sn.Color.new(1.0),
		32
	)
end
