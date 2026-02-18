local Impl = {}
Impl.__index = Impl
Impl.new = function(): GUIManager
	local self = setmetatable(
		{} :: {
			drawables: { any },
			index: number,
			add: (self: GUIManager, drawable: any) -> nil,
			update: (self: GUIManager) -> nil,
			draw: (self: GUIManager) -> nil,
		},
		Impl :: typeof(Impl)
	)
	self.drawables = {}
	self.index = 1
	return self
end
export type GUIManager = typeof(Impl.new())

Impl.add = function(self: GUIManager, drawable)
	self.drawables[self.index] = drawable
	self.index = self.index + 1
end
Impl.update = function(self: GUIManager)
	self.index = 1
end
Impl.draw = function(self: GUIManager)
	for i = 1, self.index - 1 do
		if self.drawables[i] ~= nil then
			self.drawables[i].draw()
		end
	end
end

return Impl
