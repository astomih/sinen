local sn = require("@sinen")
local font: sn.Font = sn.Font.new()
font:load(32)

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	sn.Graphics.drawText(
		"Hello World!",
		sn.TextStyle.new(font, sn.Color.new(1.0), 32),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
end
