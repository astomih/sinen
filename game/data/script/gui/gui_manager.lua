local function gui_manager()
  local object = {
    drawers = {},
    current_drawer_pos = 1,
    textures = {},
    current_texture_pos = 1,
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
      for i = 1, self.current_drawer_pos - 1 do
        self.drawers[i]:draw()
      end
      self.current_drawer_pos = 1
      self.current_texture_pos = 1
    end,
  }
  for i = 1, 100 do
    object.drawers[i] = drawui(texture())
    object.textures[i] = texture()
    object.textures[i + 100] = texture()
  end
  return object
end

return gui_manager
