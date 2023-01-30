local bullet = require "bullet"
local effect = require "effect"
-- Orbit canon
local orbit = function(player)
  local object = {
    drawer = {},
    owner = player,
    texture = {},
    aabb = {},
    model = {},
    bullets = {},
    bullet_time = 0.3,
    bullet_timer = 0,
    efk = {},
    efks = {},
    setup = function(self)
      self.model = model()
      self.model:load("orbit1.sim", "orbit1")
      self.texture = DEFAULT_TEXTURE
      self.drawer = draw3d(self.texture)
      self.drawer.scale = vector3(0.125, 0.125, 0.125)
      self.drawer.vertex_name = "orbit1"

    end,
    update = function(self, map_draw3ds)
      self.drawer.position = self.owner.drawer.position
      self.drawer.position.x = self.drawer.position.x - 5.0
      self.drawer.position.z = 5.0
      self.drawer.rotation = self.owner.drawer.rotation
      self.bullet_timer = self.bullet_timer + delta_time
      if self.bullet_timer >
          self.bullet_time then
        local b = bullet(map_draw3ds)
        b:setup(self.drawer)
        b.drawer.rotation.z = b.drawer.rotation.z + 90
        table.insert(self.bullets, b)
        self.bullet_timer = 0.0
      end
      for i, v in ipairs(self.bullets) do
        v:update()
        if v.current_time > v.life_time then
          local efk = effect()
          efk:setup()
          for j = 1, efk.max_particles do
            efk.worlds[j].position = v.drawer.position:copy()
          end
          efk:play()
          table.insert(self.efks, efk)
          table.remove(self.bullets, i)
        end
      end
      for i, v in ipairs(self.efks) do
        v:update()
        if v.is_stop then table.remove(self.efks, i) end
      end

    end,
    draw = function(self)
      for i, v in ipairs(self.bullets) do
        v:draw()
      end
      self.drawer:draw()
    end
  }
  return object

end
return orbit
