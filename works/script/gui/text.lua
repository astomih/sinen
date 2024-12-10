local function text()
  local object = {
    font = {},
    drawer = {},
    color = color(1, 1, 1, 0.9),
    show = function(self, text, pos, scale)
      local texture = GUI_MANAGER:get_texture()
      self.drawer = drawui(texture)
      self.font:render_text(texture, text, self.color)
      self.drawer.scale = texture:size()
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
    end,
  }
  object.font = font()
  object.font:load(DEFAULT_FONT_NAME, 32)
  object.drawer = drawui(texture())

  return object
end

return text
