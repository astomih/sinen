---@class GUIManager
---@field drawables any
---@field index number
---@field add fun(self: GUIManager, drawable: any)
---@field update fun(self: GUIManager)
---@field draw fun(self: GUIManager)
local GUIManager = {}

GUIManager.__index = GUIManager

GUIManager.new = function()
    local self = setmetatable({}, GUIManager)
    self.drawables = {}
    self.index = 1
    return self
end
---@param self GUIManager
---@param drawable any
GUIManager.add = function(self, drawable)
    self.drawables[self.index] = drawable
    self.index = self.index + 1
end
---@param self GUIManager
GUIManager.update = function(self)
    self.index = 1
end
---@param self GUIManager
GUIManager.draw = function(self)
    for i = 1, self.index - 1 do
        if self.drawables[i] ~= nil then
            self.drawables[i].draw()
        end
    end
end

return GUIManager
