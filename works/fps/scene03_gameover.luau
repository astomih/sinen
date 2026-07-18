local sn = require("@sinen")
local G = require("./global")
local scene_switcher = require("./scene_switcher").new()
scene_switcher:start("")
sn.Mouse.hideCursor(false)
sn.Mouse.setRelative(false)

function update()
	if scene_switcher.flag then
		scene_switcher:update()
		return
	end
	if sn.Mouse.isPressed(sn.Mouse.LEFT) then
		scene_switcher:start("main.luau")
	end
end

function draw()
	sn.Graphics.drawText(
		"GAME OVER",
		sn.TextStyle.new(G.FONT_DEFAULT64, sn.Color.new(1, 0.25, 0.25, 1), 64),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
	scene_switcher:draw()
end
