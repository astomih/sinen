local sn = require("@sinen")
local G = require("./global")
local scene_switcher = require("./scene_switcher").new()
G.NOW_STAGE = 1
scene_switcher:start("")
sn.Mouse.hideCursor(false)
sn.Mouse.setRelative(false)

function update()
	if scene_switcher.flag then
		scene_switcher:update()
		return
	end
	if sn.Mouse.isPressed(sn.Mouse.LEFT) then
		scene_switcher:start("main")
	end
end

function draw()
	sn.Graphics.drawText(
		"STAGE CLEAR",
		G.FONT_DEFAULT64,
		G.FONT_DEFAULT64:region("STAGE CLEAR", 64, sn.Pivot.Center, sn.Window.center()):topLeft(),
		sn.Color.new(1),
		64
	)
	scene_switcher:draw()
end
