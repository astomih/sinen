local function scroll()
  local object = {
    drawer = {},
    max_drawer = {},
    positions = {},
    current_pos = 1,
    add = function(self, position)
      self.positions[self.current_pos] = position
      self.current_pos = self.current_pos + 1
    end,
    prev_mpos = {},
    t1 = {},
    t2 = {},
    pos = nil,
    show = function(self, pos, scale)
      self.drawer = drawui(self.t2)
      self.drawer.scale = vector2(scale.x, scale.y * 0.1)
      if self.pos == nil then
        self.pos = vector2(pos.x, pos.y)
      end
      self.drawer.position = self.pos
      self.max_drawer = drawui(self.t1)
      self.max_drawer.scale = scale
      self.max_drawer.position = pos
      -- Mouse
      local mpos = mouse.position_on_scene()
      mpos.x = mpos.x - scene.center().x
      mpos.y = -(mpos.y - scene.center().y)
      if mouse.is_down(mouse.LEFT) then
        self.pos.y = mpos.y
        if mpos.x >= pos.x - scale.x / 2
            and
            mpos.x <= pos.x + scale.x / 2
            and
            mpos.y >= pos.y - scale.y * 0.1 / 2
            and
            mpos.y <= pos.y + scale.y * 0.1 / 2
        then
          for i, v in ipairs(self.positions) do
            v.y = mpos.y - self.prev_mpos.y + v.y
          end
        end
      else
      end
      self.prev_mpos = vector2(mpos.x, mpos.y)
      self.positions = {}
      self.current_pos = 1
      GUI_MANAGER:add(self.drawer)
      GUI_MANAGER:add(self.max_drawer)
    end,
  }
  object.drawer = drawui(texture())
  object.t1 = texture()
  object.t1:fill_color(color(0, 0, 0, 0.5))
  object.t2 = texture()
  object.t2:fill_color(color(1, 1, 1, 0.5))

  return object
end

return scroll
