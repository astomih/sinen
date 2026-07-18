local sn = require("@sinen")
local G = require("./global")
local menu = require("./gui/menu")()
local sceneSwitcher = require("./scene_switcher").new()
G.SCORE = 0
G.NOW_STAGE = 1
menu:setup()
sceneSwitcher:start("")

function update()
	if sceneSwitcher.flag then
		sceneSwitcher:update()
		return
	end
	menu:update()
	if menu.hide then
		if sn.Mouse.isPressed(sn.Mouse.LEFT) then
			sceneSwitcher:start("scene01_stage.luau")
		end
	end
end

function draw()
	sn.Graphics.drawText(
		"FPS",
		sn.TextStyle.new(G.FONT_DEFAULT64, sn.Color.new(1), 64),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
	)
	sn.Graphics.drawText(
		"CLICK TO START",
		sn.TextStyle.new(G.FONT_DEFAULT32, sn.Color.new(1, 1, 1, sn.Periodic.sin0_1(2.0, sn.Time.seconds())), 32),
		sn.TextTransform.new(sn.Window.center() + sn.Vec2.new(0, 64 * 3.0), 0.0, sn.Pivot.Center)
	)
	menu:draw()

	sceneSwitcher:draw()
end
