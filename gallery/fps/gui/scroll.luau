local G = require("../global")
local function scroll()
	local object = {
		positions = {},
		current_pos = 1,
		add = function(self: { current_pos: any, positions: { any } }, position)
			self.positions[self.current_pos] = position
			self.current_pos = self.current_pos + 1
		end,
		prev_mpos = {},
		t1 = {},
		t2 = {},
		pos = Vec2.new(0, 0),
		rect = {},
		maxRect = {},
		is_drag = false,
		initial = true,
		show = function(self, pos, scale)
			if self.initial then
				self.pos = Vec2.new(pos.x, pos.y)
				self.initial = false
			end
			self.rect = Rect.new(self.pos, Vec2.new(scale.x, scale.y * 0.1))
			self.maxRect = Rect.new(pos, scale)
			-- Mouse
			local mpos = Mouse.getPositionOnScene()
			if not self.is_drag and Mouse.isPressed(Mouse.LEFT) then
				if self.rect:intersectsRect(Rect.new(mpos, Vec2.new(1))) then
					self.is_drag = true
				end
			end
			if Mouse.isReleased(Mouse.LEFT) then
				self.is_drag = false
			end
			if self.is_drag then
				self.pos.y = mpos.y
				if self.pos.y > self.maxRect:topCenter().y then
					self.pos.y = self.maxRect:topCenter().y - self.rect.height * 0.5
				end
				if self.pos.y < self.maxRect:bottomCenter().y then
					self.pos.y = self.maxRect:bottomCenter().y + self.rect.height * 0.5
				end
				for i, v in ipairs(self.positions) do
					v.y = mpos.y + v.y
				end
			end
			self.prev_mpos = Vec2.new(mpos.x, mpos.y)
			self.positions = {}
			self.current_pos = 1
			G.GUI_MANAGER:add({
				draw = function()
					Graphics.drawImage(self.t1, self.maxRect)
				end,
			})
			G.GUI_MANAGER:add({

				draw = function()
					Graphics.drawImage(self.t2, self.rect)
				end,
			})
		end,
	}
	object.rect = Rect.new(0, 0, 0, 0)
	object.t1 = Texture.new()
	object.t1:fill(Color.new(0, 0, 0, 0.5))
	object.t2 = Texture.new()
	object.t2:fill(Color.new(1, 1, 1, 0.5))

	return object
end

return scroll
