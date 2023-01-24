local function camera_controller()
  local object = {
    position = vector3(0, 0, 0),
    target = vector3(0, 0, 0),
    up = vector3(0, 0, 1),
    prev_boost = false,
    is_tracking_player = false,
    player = {},
    setup = function(self, player)
      self.player = player
      self.position = vector3(self.player.drawer.position.x,
        self.player.drawer.position.y - 0.5,
        self.player.drawer.position.z + 15)
      self.target = vector3(self.player.drawer.position.x,
        self.player.drawer.position.y,
        self.player.drawer.position.z)
    end,
    update = function(self)
      local t = delta_time * 15
      self.position.x = self.position.x + (self.player.drawer.position.x - self.position.x) * t
      self.position.y = self.position.y + (self.player.drawer.position.y - self.position.y) * t
      self.target = vector3(self.position.x,
        self.position.y + 0.5,
        self.position.z - 15)
      scene.main_camera():lookat(self.position, self.target, vector3(0, 0, 1))
    end
  }
  return object
end

return camera_controller
