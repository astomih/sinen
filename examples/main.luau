local sn = require("@sinen")
local G = require("./global")

local exampleDirs = sn.Filesystem.enumerateDirectory(".")
local numExamples = 17
function setup()
	sn.Graphics.resetGraphicsPipeline()
end

local buttonSize = sn.Vec2.new(360, 20)
local buttonStep = 48.0
local offset = (numExamples - 1) * buttonStep
function update() end

local function centeredRect(pos: sn.Vec2, size: sn.Vec2): sn.Rect
	return sn.Rect.new(sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y), size)
end

function draw()
	sn.Gui.setFont(G.font)
	sn.Gui.setFontSize(24)
	sn.Graphics.drawText(
		"Example Launcher",
		G.font,
		G.font
			:region("Example Launcher", 24, sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(0, -(offset + 10)))
			:topLeft(),
		sn.Color.new(1, 1, 1, 1),
		24
	)
	for i = 1, numExamples do
		if sn.Gui.button(exampleDirs[i], centeredRect(sn.Vec2.new(0, (i - 1) * -buttonStep + offset), buttonSize)) then
			sn.Script.load("main", exampleDirs[i])
		end
	end
end
