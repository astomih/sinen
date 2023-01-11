-- Orbit canon
local orbit = function(player)
  local object = {
    drawer = {},
    owner = player,
    texture = {},
    aabb = {},
    model = {},
    bullets = {},
    setup = function(self)
      self.model = model()
      self.model:load("orbit1.sim", "orbit1")
      self.texture = tex
      self.drawer = draw3d(self.texture)
      self.drawer.scale = vector3(0.125, 0.125, 0.125)
      self.drawer.vertex_name = "orbit1"

    end,
    update = function(self)
      self.drawer.position = self.owner.drawer.position
      self.drawer.position.x = self.drawer.position.x - 5.0

    end,
    draw = function(self)
      self.drawer:draw()

    end
  }
  return object

end
return orbit
