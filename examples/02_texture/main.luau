local sn = require("@sinen")
local image: sn.Texture = sn.Texture.new()

local function rectFromCenter(pos: sn.Vec2, size: sn.Vec2): sn.Rect
	local half = sn.Graphics.getCamera2D():half()
	return sn.Rect.new(sn.Pivot.Center, sn.Vec2.new(half.x + pos.x, half.y - pos.y), size)
end

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
	sn.Graphics.drawImage(image, rectFromCenter(sn.Vec2.new(0), image:size()))
end
