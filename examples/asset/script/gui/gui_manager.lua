---@class gui_manager
---@field drawers any
---@field current_drawer_pos number
---@field add fun(self: gui_manager, drawer: any)
---@field update fun(self: gui_manager)
---@field draw fun(self: gui_manager)
---@return gui_manager
local function gui_manager()
    local object = {
        drawers = {},
        current_drawer_pos = 1,
        ---@param self gui_manager
        ---@param drawer any
        add = function(self, drawer)
            self.drawers[self.current_drawer_pos] = drawer
            self.current_drawer_pos = self.current_drawer_pos + 1
        end,
        ---@param self gui_manager
        update = function(self)
            self.current_drawer_pos = 1
            self.current_texture_pos = 1
        end,
        ---@param self gui_manager
        draw = function(self)
            for i = 1, self.current_drawer_pos - 1 do
                if self.drawers[i] ~= nil then
                    self.drawers[i].draw()
                end
            end
        end
    }
    return object
end

return gui_manager
