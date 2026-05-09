local sn = require("@sinen")
local image: sn.Texture = sn.Texture.new()

function setup()
	-- Create a image
	image:load("logo.png")
end

function update()
	if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
		sn.Script.load("main", ".")
	end
end

function draw()
	-- Draw image
	sn.Graphics.drawImage(image, sn.Rect.new(sn.Pivot.Center, sn.Graphics.getCamera2D():center(), image:size()))
end
