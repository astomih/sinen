local G = require("../global")
local Button = {}

Button.__index = Button
Button.new = function(): Button
	local self = setmetatable(
		{} :: {
			bgColor: Color,
			show: (self: Button, text: string, pos: Vec2, scale: Vec2) -> nil,
		},
		Button
	)
	self.bgColor = Color.new(0.7, 0.2, 0.2, 1)
	return self
end
export type Button = typeof(Button.new())

Button.show = function(self: Button, text: string, pos: Vec2, scale: Vec2)
	-- Mouse in _?
	local mpos = Mouse.getPositionOnScene()
	if
		mpos.x >= pos.x - scale.x / 2
		and mpos.x <= pos.x + scale.x / 2
		and mpos.y >= pos.y - scale.y / 2
		and mpos.y <= pos.y + scale.y / 2
	then
		G.GUI_MANAGER:add({
			draw = function()
				Graphics.drawRect(Rect.new(pos, scale), Color.new(0.2, 0.7, 0.2, 1))
			end,
		})
		if Mouse.isPressed(Mouse.LEFT) then
			G.GUI_MANAGER:add({
				draw = function()
					Graphics.drawRect(Rect.new(pos, scale), Color.new(0.2, 0.2, 0.7, 1))
				end,
			})
			return true
		end
	else
		G.GUI_MANAGER:add({
			draw = function()
				Graphics.drawRect(Rect.new(pos, scale), self.bgColor)
			end,
		})
	end
	G.GUI_MANAGER:add({
		draw = function()
			Graphics.drawText(text, G.FONT_DEFAULT64, pos, Color.new(1), 24)
		end,
	})

	return false
end

return Button
