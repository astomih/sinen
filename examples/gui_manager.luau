local Impl = {}
Impl.__index = Impl
Impl.new = function()
	local self = setmetatable({}, Impl)
	self.drawables = {}
	self.index = 1
	return self
end
Impl.add = function(self, drawable)
	self.drawables[self.index] = drawable
	self.index = self.index + 1
end
Impl.update = function(self)
	self.index = 1
end
Impl.draw = function(self)
	for i = 1, self.index - 1 do
		if self.drawables[i] ~= nil then
			self.drawables[i].draw()
		end
	end
end

return Impl
