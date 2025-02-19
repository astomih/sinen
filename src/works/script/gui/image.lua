local function image()
  local object = {
    drawer = {},
    show = function(self, texture, pos, scale)
      self.drawer = draw2d(texture)
      self.drawer.scale = scale
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
    end,
  }
  object.drawer = draw2d(texture())

  return object
end

return image
