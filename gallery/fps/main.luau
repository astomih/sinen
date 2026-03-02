local sn = require("@sinen")
local G = require("./global")
local menu = require("./gui/menu")()
local sceneSwitcher = require("./scene_switcher").new()
sn.Graphics.getCamera2D():resize(sn.Vec2.new(1280, 720))
G.SCORE = 0
G.NOW_STAGE = 1
menu:setup()
sceneSwitcher:start("")

function update()
	G.GUI_MANAGER:update()
	if sceneSwitcher.flag then
		sceneSwitcher:update()
		return
	end
	menu:update()
	if menu.hide then
		if sn.Mouse.isPressed(sn.Mouse.LEFT) then
			sceneSwitcher:start("scene01_stage")
		end
	end
end

function draw()
	sn.Graphics.drawText("FPS", G.FONT_DEFAULT64, sn.Vec2.new(0, 0), sn.Color.new(1), 64)
	sn.Graphics.drawText(
		"CLICK TO START",
		G.FONT_DEFAULT32,
		sn.Vec2.new(0, -64 * 3.0),
		sn.Color.new(1, 1, 1, sn.Periodic.sin0_1(2.0, sn.Time.seconds())),
		32
	)
	menu:draw()

	sceneSwitcher:draw()
	G.GUI_MANAGER:draw()
end
