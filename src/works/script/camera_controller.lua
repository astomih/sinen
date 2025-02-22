local function camera_controller()
  local object = {
    position = vector3(0, 0, 0),
    target = vector3(0, 0, 0),
    up = vector3(0, 0, 1),
    prev_boost = false,
    is_tracking_player = false,
    player = {},
    py = 5,
    pz = 12,
    track_speed = 15,
    setup = function(self, player)
      self.player = player
      self.position = vector3(self.player.drawer.position.x,
        self.player.drawer.position.y - self.py,
        self.player.drawer.position.z + self.pz)
      self.target = vector3(self.player.drawer.position.x,
        self.player.drawer.position.y,
        self.player.drawer.position.z)
    end,
    update = function(self)
      local t = scene.delta_time() * self.track_speed
      local dx = (self.player.drawer.position.x - self.target.x)
      self.position.x = self.position.x + dx * t
      local dy = (self.player.drawer.position.y - self.target.y)
      self.position.y = self.position.y + dy * t
      self.target = vector3(self.position.x,
        self.position.y + self.py,
        self.position.z - self.pz)
      scene.main_camera():lookat(self.position, self.target, vector3(0, 0, 1))
      local pos = vector3(self.target.x - 2.0, self.target.y - 2.0, self.target.z - 3.0)
      renderer.set_light_lookat(pos, self.target, vector3(0, 0, 1))
      renderer.set_light_ortho(-30, 25, -20, 20, -100, 100)
    end
  }
  return object
end

return camera_controller
