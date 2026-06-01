local sn = require("@sinen")
local G = require("../../global")
local sound: sn.Sound = sn.Sound.new()
sound:load("shot.wav")

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
	if sn.Keyboard.isPressed(sn.Scancode.SPACE) then
		sound:play()
	end
end

function draw()
	sn.Graphics.drawText(
		"Press SPACE key to play sound",
		sn.TextStyle.new(G.font, sn.Color.new(1), 16),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
end
