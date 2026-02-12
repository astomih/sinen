local G = require("./global")
local button = require("./gui/button").new()

local exampleDirs = Filesystem.enumerateDirectory(".")
local numExamples = 14
function setup()
	Graphics.resetGraphicsPipeline()
end

local offset = numExamples * 16.0
function update()
	G.guiManager:update()

	for i = 1, numExamples do
		if button:show(exampleDirs[i], Vec2.new(0, i * -32.0 + offset), Vec2.new(300, 32)) then
			Script.load("main", exampleDirs[i])
		end
	end
end

function draw()
	Graphics.drawText("Example Launcher", G.font, Vec2.new(0, offset + 10), Color.new(1, 1, 1, 1), 24)
	G.guiManager:draw()
end
