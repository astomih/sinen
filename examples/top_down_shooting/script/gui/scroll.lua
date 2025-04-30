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
    pos = Vec2(0, 0),
    is_drag = false,
    initial = true,
    show = function(self, pos, scale)
      self.drawer = Draw2D(self.t2)
      self.drawer.scale = Vec2(scale.x, scale.y * 0.1)
      if self.initial then
        self.pos = Vec2(pos.x, pos.y)
        self.initial = false
      end
      self.drawer.position = self.pos
      self.max_drawer = Draw2D(self.t1)
      self.max_drawer.scale = scale
      self.max_drawer.position = pos
      -- Mouse
      local mpos = mouse.position_on_scene()
      if not self.is_drag and mouse.is_pressed(mouse.LEFT) then
        local dpos = self.drawer.position
        local dscale = self.drawer.scale
        if mpos.x >= dpos.x - dscale.x / 2
            and
            mpos.x <= dpos.x + dscale.x / 2
            and
            mpos.y >= dpos.y - dscale.y / 2
            and
            mpos.y <= dpos.y + dscale.y / 2
        then
          self.is_drag = true
        end
      end
      if (mouse.is_released(mouse.LEFT)) then self.is_drag = false end
      if self.is_drag then
        self.pos.y = mpos.y
        if self.pos.y < self.max_drawer.position.y - self.max_drawer.scale.y / 2 + self.drawer.scale.y / 2 then
          self.pos.y = self.max_drawer.position.y - self.max_drawer.scale.y / 2 + self.drawer.scale.y / 2
        end
        if self.pos.y > self.max_drawer.position.y + self.max_drawer.scale.y / 2 - self.drawer.scale.y / 2 then
          self.pos.y = self.max_drawer.position.y + self.max_drawer.scale.y / 2 - self.drawer.scale.y / 2
        end
        for i, v in ipairs(self.positions) do
          v.y = mpos.y + v.y
        end
      end
      self.prev_mpos = Vec2(mpos.x, mpos.y)
      self.positions = {}
      self.current_pos = 1
      GUI_MANAGER:add(self.drawer)
      GUI_MANAGER:add(self.max_drawer)
    end,
  }
  object.drawer = Draw2D(Texture())
  object.t1 = Texture()
  object.t1:fill_color(Color(0, 0, 0, 0.5))
  object.t2 = Texture()
  object.t2:fill_color(Color(1, 1, 1, 0.5))

  return object
end

return scroll
