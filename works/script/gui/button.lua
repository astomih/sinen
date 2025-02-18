require("gui/global")
local function button()
  local object = {
    font = {},
    back_drawer = {},
    drawer = {},
    bg_color = color(0.7, 0.2, 0.2, 1),
    fg_color = color(1, 1, 1, 0.9),
    show = function(self, text, pos, scale)
      self.font:resize(32 * UI_SCALE)
      local back_texture = GUI_MANAGER:get_texture()
      local texture = GUI_MANAGER:get_texture()
      back_texture:fill_color(self.bg_color)
      self.back_drawer = draw2d(back_texture)
      self.back_drawer.position = pos
      self.back_drawer.scale = vector2(scale.x * UI_SCALE, scale.y * UI_SCALE)
      GUI_MANAGER:add(self.back_drawer)
      self.drawer = draw2d(texture)
      self.font:render_text(texture, text, self.fg_color)
      self.drawer.scale = texture:size()
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
      -- Mouse in button?
      local mpos = mouse.position()
      mpos.x = mpos.x - window.center().x
      mpos.y = -(mpos.y - window.center().y)
      if mpos.x >= pos.x - scale.x / 2
          and
          mpos.x <= pos.x + scale.x / 2
          and
          mpos.y >= pos.y - scale.y / 2
          and
          mpos.y <= pos.y + scale.y / 2
      then
        back_texture:fill_color(color(0.2, 0.7, 0.2, 1))
        if mouse.is_pressed(mouse.LEFT) then
          back_texture:fill_color(color(0.2, 0.2, 0.7, 1))
          return true
        end
      else
        back_texture:fill_color(self.bg_color)
      end
      return false
    end,
  }
  object.back_drawer = draw2d(texture())
  object.font = font()
  object.font:load(DEFAULT_FONT_NAME, 32)
  object.drawer = draw2d(texture())

  return object
end

return button
