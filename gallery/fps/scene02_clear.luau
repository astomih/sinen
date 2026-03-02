local sn = require("@sinen")
local G = require("./global")
local scene_switcher = require("./scene_switcher").new()
G.NOW_STAGE = 1
scene_switcher:start("")
sn.Mouse.hideCursor(false)
sn.Mouse.setRelative(false)

function update()
	G.GUI_MANAGER:update()
	if scene_switcher.flag then
		scene_switcher:update()
		return
	end
	if sn.Mouse.isPressed(sn.Mouse.LEFT) then
		scene_switcher:start("main")
	end
end

function draw()
	sn.Graphics.drawText("STAGE CLEAR", G.FONT_DEFAULT64, sn.Vec2.new(0), sn.Color.new(1), 64)
	scene_switcher:draw()
	G.GUI_MANAGER:draw()
end
