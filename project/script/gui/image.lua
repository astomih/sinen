local function image()
  local object = {
    drawer = {},
    show = function(self, texture, pos, scale)
      self.drawer = drawui(texture)
      self.drawer.scale = scale
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
    end,
  }
  object.drawer = drawui(texture())

  return object
end

return image
