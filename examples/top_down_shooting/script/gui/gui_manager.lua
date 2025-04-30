local function gui_manager()
  local object = {
    drawers = {},
    current_drawer_pos = 1,
    textures = {},
    current_texture_pos = 1,
    max_object = 1000,
    add_object_pool = function(self, new_size)
      for i = self.max_object, self.max_object + new_size do
        self:add(self.drawers[i])
        self:add(self.textures[i])
        self:add(self.textures[i + new_size])
      end
    end,
    add = function(self, drawer)
      self.drawers[self.current_drawer_pos].position = drawer.position
      self.drawers[self.current_drawer_pos].scale    = drawer.scale
      self.drawers[self.current_drawer_pos].texture  = drawer.texture
      self.current_drawer_pos                        = self.current_drawer_pos + 1
    end,
    get_texture = function(self)
      self.current_texture_pos = self.current_texture_pos + 1
      return self.textures[self.current_texture_pos - 1]
    end,
    update = function(self)
      self.current_drawer_pos = 1
      self.current_texture_pos = 1
    end,
    draw = function(self)
      for i = 1, self.current_drawer_pos - 1 do
        self.drawers[i]:draw()
      end
    end
  }
  for i = 1, object.max_object do
    object.drawers[i] = Draw2D(Texture())
    object.textures[i] = Texture()
    object.textures[i + object.max_object] = Texture()
  end
  return object
end

return gui_manager
