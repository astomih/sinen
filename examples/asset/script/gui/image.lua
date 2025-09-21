local function image()
  local object = {
    drawer = {},
    show = function(self, texture, pos, scale)
      self.drawer = sn.Draw2D(texture)
      self.drawer.scale = scale
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
    end,
  }
  object.drawer = sn.Draw2D(sn.Texture())

  return object
end

return image
