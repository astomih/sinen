local sn = require("@sinen")
local G = require("./global")

local galleryDirs = {
	"fps",
	"block_place",
	"neon_orbit",
	"shader_toy",
}

local numGallerys = #galleryDirs
function setup()
	sn.Graphics.resetGraphicsPipeline()
end

local buttonSize = sn.Vec2.new(360, 40)
local buttonStep = 48.0
local offset = (numGallerys - 1) * buttonStep * 0.5

function update()
	if sn.Keyboard.isPressed(sn.Scancode.ESCAPE) then
		sn.Script.load("/main.luau")
	end
end

local function centeredRect(pos: sn.Vec2, size: sn.Vec2): sn.Rect
	return sn.Rect.new(sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y), size)
end

function draw()
	sn.Gui.setFont(G.font)
	sn.Gui.setFontSize(24)
	sn.Graphics.drawText(
		"Gallery Launcher",
		sn.TextStyle.new(G.font, sn.Color.new(1, 1, 1, 1), 24),
		sn.TextTransform.new(sn.Window.center() + sn.Vec2.new(0, -(offset + 10)), 0.0, sn.Pivot.Center)
	)
	for i = 1, numGallerys do
		if sn.Gui.button(galleryDirs[i], centeredRect(sn.Vec2.new(0, (i - 1) * -buttonStep + offset), buttonSize)) then
			sn.Script.load("/gallery/" .. galleryDirs[i] .. "/main.luau")
		end
	end
end
