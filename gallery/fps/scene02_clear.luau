local G = require("./global")
local scene_switcher = require("./scene_switcher").new()
G.NOW_STAGE = 1
scene_switcher:start("")
Mouse.hideCursor(false)
Mouse.setRelative(false)

function update()
	G.GUI_MANAGER:update()
	if scene_switcher.flag then
		scene_switcher:update()
		return
	end
	if Mouse.isPressed(Mouse.LEFT) then
		scene_switcher:start("main")
	end
end

function draw()
	Graphics.drawText("STAGE CLEAR", G.FONT_DEFAULT64, Vec2.new(0), Color.new(1), 64)
	scene_switcher:draw()
	G.GUI_MANAGER:draw()
end
