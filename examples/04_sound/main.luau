local sn = require("@sinen")
local G = require("../global")
local sound: sn.Sound = sn.Sound.new()
sound:load("shot.wav")

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
	if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
		sound:play()
	end
end

function draw()
	sn.Graphics.drawText(
		"Press SPACE key to play sound",
		G.font,
		G.font
			:region("Press SPACE key to play sound", 16, sn.Pivot.Center, sn.Graphics.getCamera2D():center())
			:topLeft(),
		sn.Color.new(1),
		16
	)
end
