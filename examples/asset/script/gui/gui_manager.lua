---@class gui_manager
---@field drawers Draw2D[]
---@field current_drawer_pos number
---@field textures Texture[]
---@field current_texture_pos number
---@field max_object number
---@field add_object_pool fun(self: gui_manager, new_size: number)
---@field add fun(self: gui_manager, drawer: Draw2D)
---@field get_texture fun(self: gui_manager): Texture
---@field update fun(self: gui_manager)
---@field draw fun(self: gui_manager)
---@return gui_manager
local function gui_manager()
    local object = {
        drawers = {},
        current_drawer_pos = 1,
        textures = {},
        current_texture_pos = 1,
        max_object = 100,
        add_object_pool = function(self, new_size)
            for i = self.max_object, self.max_object + new_size do
                self:add(self.drawers[i])
                self:add(self.textures[i])
                self:add(self.textures[i + new_size])
            end
        end,
        ---@param self gui_manager
        ---@param drawer Draw2D
        add = function(self, drawer)
            self.drawers[self.current_drawer_pos].position = drawer.position
            self.drawers[self.current_drawer_pos].scale    = drawer.scale
            self.drawers[self.current_drawer_pos].material = drawer.material
            self.current_drawer_pos                        = self.current_drawer_pos + 1
        end,
        ---@param self gui_manager
        ---@return Texture
        get_texture = function(self)
            self.current_texture_pos = self.current_texture_pos + 1
            return self.textures[self.current_texture_pos - 1]
        end,
        ---@param self gui_manager
        update = function(self)
            self.current_drawer_pos = 1
            self.current_texture_pos = 1
        end,
        ---@param self gui_manager
        draw = function(self)
            for i = 1, self.current_drawer_pos - 1 do
                sn.Graphics.draw2d(self.drawers[i])
            end
        end
    }
    for i = 1, object.max_object do
        object.drawers[i] = sn.Draw2D(sn.Texture())
        object.textures[i] = sn.Texture()
        object.textures[i + object.max_object] = sn.Texture()
    end
    return object
end

return gui_manager
