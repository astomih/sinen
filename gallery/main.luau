local sn = require("@sinen")
local G = require("./global")
local button = require("./gui/button").new()

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

local offset = numGallerys * 16.0
function update()
	G.guiManager:update()

	for i = 1, numGallerys do
		if button:show(galleryDirs[i], sn.Vec2.new(0, i * -32.0 + offset), sn.Vec2.new(300, 32)) then
			sn.Script.load("main", galleryDirs[i])
		end
	end
end

function draw()
	sn.Graphics.drawText(
		"Gallery Launcher",
		G.font,
		G.textTopLeftFromCenter("Gallery Launcher", G.font, sn.Vec2.new(0, offset + 10), 24),
		sn.Color.new(1, 1, 1, 1),
		24
	)
	G.guiManager:draw()
end
