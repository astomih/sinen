require("gui/global")
local function button()
  local object = {
    font = {},
    back_drawer = {},
    drawer = {},
    bg_color = Color(0.7, 0.2, 0.2, 1),
    fg_color = Color(1, 1, 1, 0.9),
    show = function(self, text, pos, scale)
      self.font:resize(32 * UI_SCALE)
      local back_texture = GUI_MANAGER:get_texture()
      local texture = GUI_MANAGER:get_texture()
      back_texture:fill_color(self.bg_color)
      self.back_drawer = Draw2D(back_texture)
      self.back_drawer.position = pos
      self.back_drawer.scale = Vec2(scale.x * UI_SCALE, scale.y * UI_SCALE)
      GUI_MANAGER:add(self.back_drawer)
      self.drawer = Draw2D(texture)
      self.font:render_text(texture, text, self.fg_color)
      self.drawer.scale = texture:size()
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
      -- Mouse in button?
      local mpos = mouse.position_on_scene()
      if mpos.x >= pos.x - scale.x / 2
          and
          mpos.x <= pos.x + scale.x / 2
          and
          mpos.y >= pos.y - scale.y / 2
          and
          mpos.y <= pos.y + scale.y / 2
      then
        back_texture:fill_color(Color(0.2, 0.7, 0.2, 1))
        if mouse.is_pressed(mouse.LEFT) then
          back_texture:fill_color(Color(0.2, 0.2, 0.7, 1))
          return true
        end
      else
        back_texture:fill_color(self.bg_color)
      end
      return false
    end,
  }
  object.back_drawer = Draw2D(Texture())
  object.font = Font()
  object.font:load(DEFAULT_FONT_NAME, 32)
  object.drawer = Draw2D(Texture())

  return object
end

return button
