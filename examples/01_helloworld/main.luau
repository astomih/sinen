local sn = require("@sinen")
local font: sn.Font = sn.Font.new()
font:load(32)

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
end

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		font,
		font:region("Hello World!", 32, sn.Pivot.Center, sn.Window.center()):topLeft(),
		sn.Color.new(1.0),
		32
	)
end
