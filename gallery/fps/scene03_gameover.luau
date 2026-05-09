local sn = require("@sinen")
local G = require("./global")
local scene_switcher = require("./scene_switcher").new()
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
	sn.Graphics.drawText(
		"GAME OVER",
		G.FONT_DEFAULT64,
		G.FONT_DEFAULT64:region("GAME OVER", 64, sn.Pivot.Center, sn.Graphics.getCamera2D():center()):topLeft(),
		sn.Color.new(1, 0.25, 0.25, 1),
		64
	)
	scene_switcher:draw()
	G.GUI_MANAGER:draw()
end
