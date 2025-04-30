local function camera_controller()
  local object = {
    position = Vec3(0, 0, 0),
    target = Vec3(0, 0, 0),
    up = Vec3(0, 0, 1),
    prev_boost = false,
    is_tracking_player = false,
    player = {},
    py = 5,
    pz = 12,
    track_speed = 15,
    setup = function(self, player)
      self.player = player
      self.position = Vec3(self.player.drawer.position.x,
        self.player.drawer.position.y - self.py,
        self.player.drawer.position.z + self.pz)
      self.target = Vec3(self.player.drawer.position.x,
        self.player.drawer.position.y,
        self.player.drawer.position.z)
    end,
    update = function(self)
      local t = scene.delta_time() * self.track_speed
      local dx = (self.player.drawer.position.x - self.target.x)
      self.position.x = self.position.x + dx * t
      local dy = (self.player.drawer.position.y - self.target.y)
      self.position.y = self.position.y + dy * t
      self.target = Vec3(self.position.x,
        self.position.y + self.py,
        self.position.z - self.pz)
      scene.camera():lookat(self.position, self.target, Vec3(0, 0, 1))
    end
  }
  return object
end

return camera_controller
