import("global")
local button = require("gui/button").new()
GUI_MANAGER = require("gui/gui_manager")()

local exampleDirs = sn.FileSystem.enumerateDirectory(".")
local numExamples = 13
sn.Graphics.bindPipeline(sn.BuiltinPipelines.get2D())
function Setup()
end

local offset = numExamples * 16.0
function Update()
    GUI_MANAGER:update()

    for i = 1, numExamples do
        if button:show(exampleDirs[i], sn.Vec2.new(0, i * -32.0 + offset), sn.Vec2.new(300, 32)) then
            sn.Script.load("main", exampleDirs[i])
        end
    end
end

function Draw()
    sn.Graphics.drawText("Example Launcher", FONT, sn.Vec2.new(0, offset), sn.Color.new(1, 1, 1, 1), 24)
    GUI_MANAGER:draw()
end
