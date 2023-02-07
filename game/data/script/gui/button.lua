local function button()
  local object = {
    font = {},
    back_drawer = {},
    drawer = {},
    show = function(self, text, pos, scale)
      local back_texture = GUI_MANAGER:get_texture()
      local texture = GUI_MANAGER:get_texture()
      back_texture:fill_color(color(0.7, 0.2, 0.2, 1))
      self.back_drawer = drawui(back_texture)
      self.back_drawer.position = pos
      self.back_drawer.scale = scale
      GUI_MANAGER:add(self.back_drawer)
      self.drawer = drawui(texture)
      self.font:render_text(texture, text, color(1, 1, 1, 0.9))
      self.drawer.scale = texture:size()
      self.drawer.position = pos
      GUI_MANAGER:add(self.drawer)
      -- Mouse in button?
      local mpos = mouse:position_on_scene()
      mpos.x = mpos.x - scene.center().x
      mpos.y = -(mpos.y - scene.center().y)
      if mpos.x >= pos.x - scale.x / 2
          and
          mpos.x <= pos.x + scale.x / 2
          and
          mpos.y >= pos.y - scale.y / 2
          and
          mpos.y <= pos.y + scale.y / 2
      then
        back_texture:fill_color(color(0.2, 0.7, 0.2, 1))
        if mouse:is_button_pressed(mouseLEFT) then
          back_texture:fill_color(color(0.2, 0.2, 0.7, 1))
          return true
        end
      else
        back_texture:fill_color(color(0.7, 0.2, 0.2, 1))
      end
      return false
    end,
  }
  object.back_drawer = drawui(texture())
  object.font = font()
  object.font:load(DEFAULT_FONT_NAME, 32)
  object.drawer = drawui(texture())

  return object
end

return button