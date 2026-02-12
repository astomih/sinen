local G = require("../global")
local sound: Sound = Sound.new()
sound:load("shot.wav")

function update()
	if Keyboard.isPressed(Keyboard.ESCAPE) then
		Script.load("main", ".")
	end
	if Keyboard.isPressed(Keyboard.SPACE) then
		sound:play()
	end
end

function draw()
	Graphics.drawText("Press SPACE key to play sound", G.font, Vec2.new(0), Color.new(1), 16)
end
