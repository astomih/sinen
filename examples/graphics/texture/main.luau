local sn = require("@sinen")
local image: sn.Texture = sn.Texture.new("logo.png")

function setup()
	-- Create a image
end

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

function draw()
	-- Draw image
	sn.Graphics.drawImage(image, sn.Rect.new(sn.Pivot.Center, sn.Window.center(), image:size()))
end
