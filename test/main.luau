local sn = require("@sinen")
local G = require("./global")
local button = require("./gui/button").new()

local testDirs = {
	"imgui",
}

local numGallerys = #testDirs
function setup()
	sn.Graphics.resetGraphicsPipeline()
end

local offset = numGallerys * 16.0
function update()
	G.guiManager:update()

	for i = 1, numGallerys do
		if button:show(testDirs[i], sn.Vec2.new(0, i * -32.0 + offset), sn.Vec2.new(300, 32)) then
			sn.Script.load("main", testDirs[i])
		end
	end
end

function draw()
	sn.Graphics.drawText("Test Launcher", G.font, sn.Vec2.new(0, offset + 10), sn.Color.new(1, 1, 1, 1), 24)
	G.guiManager:draw()
end
