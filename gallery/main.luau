local G = require("./global")
local button = require("./gui/button").new()

local galleryDirs = {
	"fps",
}

local numGallerys = #galleryDirs
function setup()
	Graphics.resetGraphicsPipeline()
end

local offset = numGallerys * 16.0
function update()
	G.guiManager:update()

	for i = 1, numGallerys do
		if button:show(galleryDirs[i], Vec2.new(0, i * -32.0 + offset), Vec2.new(300, 32)) then
			Script.load("main", galleryDirs[i])
		end
	end
end

function draw()
	Graphics.drawText("Gallery Launcher", G.font, Vec2.new(0, offset + 10), Color.new(1, 1, 1, 1), 24)
	G.guiManager:draw()
end
