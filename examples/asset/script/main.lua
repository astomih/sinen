local button = require("gui/button")()
GUI_MANAGER = require("gui/gui_manager")()

local exampleDirs = sn.FileSystem.enumerate_directory(".")
local numExamples = 9


local offset = numExamples * 16.0
function update()
    GUI_MANAGER:update()

    for i = 1, numExamples do
        if button:show(exampleDirs[i], sn.Vec2(0, i * -32.0 + offset
            ), sn.Vec2(300, 32)) then
            sn.Script.load("main", exampleDirs[i])
        end
    end
end

function draw()
    sn.Graphics.bind_default_pipeline2d()
    sn.Graphics.bind_default_pipeline3d()
    sn.Graphics.draw_text("Example Launcher",
        sn.Vec2(0, offset),
        sn.Color(1, 1, 1, 1), 24)
    GUI_MANAGER:draw()
end
