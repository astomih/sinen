local sn = require("@sinen")
local G = require("../global")
local Scroll = {}
Scroll.__index = Scroll
Scroll.new = function()
	local self = setmetatable(
		{} :: {
			positions: { sn.Vec2 },
			current_pos: number,
			prev_mpos: sn.Vec2,
			t1: sn.Texture,
			t2: sn.Texture,
			pos: sn.Vec2,
			rect: sn.Rect,
			maxRect: sn.Rect,
			is_drag: boolean,
			initial: boolean,
			add: (self: Scroll, position: sn.Vec2) -> (),
			show: (self: Scroll, pos: sn.Vec2, scale: sn.Vec2) -> (),
		},
		Scroll
	)
	self.positions = {} :: { sn.Vec2 }
	self.current_pos = 1
	self.prev_mpos = sn.Vec2.new(0, 0)
	self.is_drag = false
	self.initial = true
	self.pos = sn.Vec2.new(0, 0)
	self.rect = sn.Rect.new(0, 0, 0, 0)
	self.maxRect = sn.Rect.new(0, 0, 0, 0)
	self.t1 = sn.Texture.new()
	self.t1:fill(sn.Color.new(0, 0, 0, 0.5))
	self.t2 = sn.Texture.new()
	self.t2:fill(sn.Color.new(1, 1, 1, 0.5))
	return self
end
Scroll.add = function(self: Scroll, position: sn.Vec2)
	self.positions[self.current_pos] = position
	self.current_pos = self.current_pos + 1
end
Scroll.show = function(self: Scroll, pos: sn.Vec2, scale: sn.Vec2)
	if self.initial then
		self.pos = sn.Vec2.new(pos.x, pos.y)
		self.initial = false
	end
	self.rect = sn.Rect.new(self.pos, sn.Vec2.new(scale.x, scale.y * 0.1))
	self.maxRect = sn.Rect.new(pos, scale)
	-- Mouse
	local mpos = sn.Mouse.getPositionOnScene()
	if not self.is_drag and sn.Mouse.isPressed(sn.Mouse.LEFT) then
		if self.rect:intersectsRect(sn.Rect.new(mpos, sn.Vec2.new(1))) then
			self.is_drag = true
		end
	end
	if sn.Mouse.isReleased(sn.Mouse.LEFT) then
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
	self.prev_mpos = sn.Vec2.new(mpos.x, mpos.y)
	self.positions = {}
	self.current_pos = 1
	G.GUI_MANAGER:add({
		draw = function()
			sn.Graphics.drawImage(self.t1, self.maxRect)
		end,
	})
	G.GUI_MANAGER:add({

		draw = function()
			sn.Graphics.drawImage(self.t2, self.rect)
		end,
	})
end
export type Scroll = typeof(Scroll.new())

return Scroll
