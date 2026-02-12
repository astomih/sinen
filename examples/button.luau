local Button = {}

Button.__index = Button

Button.new = function()
	local self = setmetatable({}, Button)
	self.font = Font.new()
	self.font:load(24)
	self.bg_color = Color.new(0.7, 0.2, 0.2, 1.0)
	return self
end

Button.show = function(self, text: string, pos: Vec2, scale: Vec2)
	-- Mouse in _?
	local mpos = Mouse.getPositionOnScene()
	if
		mpos.x >= pos.x - scale.x / 2
		and mpos.x <= pos.x + scale.x / 2
		and mpos.y >= pos.y - scale.y / 2
		and mpos.y <= pos.y + scale.y / 2
	then
		gGuiManager:add({
			draw = function()
				Graphics.drawRect(Rect.new(pos, scale), Color.new(0.2, 0.7, 0.2, 1))
			end,
		})
		if Mouse.isPressed(Mouse.LEFT) then
			gGuiManager:add({
				draw = function()
					Graphics.drawRect(Rect.new(pos, scale), Color.new(0.2, 0.2, 0.7, 1))
				end,
			})
			return true
		end
	else
		gGuiManager:add({
			draw = function()
				Graphics.drawRect(Rect.new(pos, scale), self.bg_color)
			end,
		})
	end
	gGuiManager:add({
		draw = function()
			Graphics.drawText(text, self.font, pos, Color.new(1), 14)
		end,
	})

	return false
end

return Button
