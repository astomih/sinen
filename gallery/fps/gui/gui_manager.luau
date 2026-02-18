local GUIManager = {}

GUIManager.__index = GUIManager

GUIManager.new = function()
	local self = setmetatable(
		{} :: {
			drawables: { any },
			index: number,
			add: (self: GUIManager, drawable: any) -> nil,
			update: (self: GUIManager) -> nil,
			draw: (self: GUIManager) -> nil,
		},
		GUIManager :: typeof(GUIManager)
	)
	self.drawables = {}
	self.index = 1
	return self
end
export type GUIManager = typeof(GUIManager.new())
GUIManager.add = function(self: GUIManager, drawable)
	self.drawables[self.index] = drawable
	self.index = self.index + 1
end
---@param self GUIManager
GUIManager.update = function(self: GUIManager)
	self.index = 1
end
---@param self GUIManager
GUIManager.draw = function(self: GUIManager)
	for i = 1, self.index - 1 do
		if self.drawables[i] ~= nil then
			self.drawables[i].draw()
		end
	end
end
return GUIManager
