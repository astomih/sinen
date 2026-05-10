local sn = require("@sinen")
local G = require("../global")
local Button = {}

Button.__index = Button
Button.new = function()
	local self = setmetatable(
		{} :: {
			bgColor: sn.Color,
			show: (self: Button, text: string, pos: sn.Vec2, scale: sn.Vec2) -> (),
		},
		Button
	)
	self.bgColor = sn.Color.new(0.7, 0.2, 0.2, 1)
	return self
end
export type Button = typeof(Button.new())

Button.show = function(self: Button, text: string, pos: sn.Vec2, scale: sn.Vec2)
	-- Mouse in _?
	local mpos = sn.Mouse.getPositionOnScene()
	if
		mpos.x >= pos.x - scale.x / 2
		and mpos.x <= pos.x + scale.x / 2
		and mpos.y >= pos.y - scale.y / 2
		and mpos.y <= pos.y + scale.y / 2
	then
		G.GUI_MANAGER:add({
			draw = function()
				sn.Graphics.drawRect(
					sn.Rect.new(sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y), scale),
					sn.Color.new(0.2, 0.7, 0.2, 1)
				)
			end,
		})
		if sn.Mouse.isPressed(sn.Mouse.LEFT) then
			G.GUI_MANAGER:add({
				draw = function()
					sn.Graphics.drawRect(
						sn.Rect.new(
							sn.Pivot.Center,
							sn.Window.center() + sn.Vec2.new(pos.x, -pos.y),
							scale
						),
						sn.Color.new(0.2, 0.2, 0.7, 1)
					)
				end,
			})
			return true
		end
	else
		G.GUI_MANAGER:add({
			draw = function()
				sn.Graphics.drawRect(
					sn.Rect.new(sn.Pivot.Center, sn.Window.center() + sn.Vec2.new(pos.x, -pos.y), scale),
					self.bgColor
				)
			end,
		})
	end
	G.GUI_MANAGER:add({
		draw = function()
			sn.Graphics.drawText(
				text,
				G.FONT_DEFAULT64,
				G.FONT_DEFAULT64
					:region(
						text,
						24,
						sn.Pivot.Center,
						sn.Window.center() + sn.Vec2.new(pos.x, -pos.y)
					)
					:topLeft(),
				sn.Color.new(1),
				24
			)
		end,
	})

	return false
end

return Button
