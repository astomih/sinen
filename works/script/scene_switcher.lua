local scene_switcher = function()
  local object = {
    texture = texture(),
    color = color(0, 0, 0, 0.0),
    drawer = {},
    time = 0.25,
    timer = 0.0,
    flag = false,
    scene_name = "",
    is_launch = false,
    setup = function(self)
      self.drawer = draw2d(self.texture)
      self.drawer.scale = window:size()
    end,
    update = function(self, draw_func)
      self.drawer.scale = window:size()
      if self.flag then
        draw_func()
        if not self.is_launch then
          if self.timer < self.time then
            self.timer = self.timer + delta_time
            local t = self.timer * (1.0 / self.time)
            if t > 1.0 then
              t = 1.0
            end
            if t < 0.0 then
              t = 1.0
            end
            self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, t))
            self.drawer:draw()
          else
            self.timer = 0
            self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, 1.0))
            self.flag = false
            self.drawer:draw()
            change_scene(self.scene_name)
          end
        else
          if self.timer > 0.0 then
            self.timer = self.timer - delta_time
            local t = self.timer * (1.0 / self.time)
            if t < 0.0 then
              t = 0.0
            end
            self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, t))
            self.drawer:draw()
          else
            self.flag = false
            self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, 0.0))
            self.timer = 0.0
            self.drawer:draw()
          end
        end
      end
    end,
    start = function(self, scene_name)
      self.is_launch = string.len(scene_name) == 0
      if self.is_launch then
        self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, 1.0))
        self.flag = false
        self.timer = self.time
      else
        self.texture:fill_color(color(self.color.r, self.color.g, self.color.b, 0.0))
        self.timer = 0.0
      end
      self.scene_name = scene_name
      self.flag = true
    end
  }
  return object
end
return scene_switcher
