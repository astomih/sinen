---@class gui_manager
---@field drawables any
---@field index number
---@field add fun(self: gui_manager, drawable: any)
---@field update fun(self: gui_manager)
---@field draw fun(self: gui_manager)
---@return gui_manager
local function gui_manager()
    local object = {
        drawables = {},
        index = 1,
        ---@param self gui_manager
        ---@param drawable any
        add = function(self, drawable)
            self.drawables[self.index] = drawable
            self.index = self.index + 1
        end,
        ---@param self gui_manager
        update = function(self)
            self.index = 1
        end,
        ---@param self gui_manager
        draw = function(self)
            for i = 1, self.index - 1 do
                local drawable = self.drawables[i]
                if drawable ~= nil then
                    drawable.draw()
                end
            end
        end
    }
    return object
end

return gui_manager

