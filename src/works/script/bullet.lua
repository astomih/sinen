local m = model()
local sound = sound()
sound:load("shot.wav")
sound:set_volume(0.3)
local effect = require "effect"

local function bullet(map_draw3ds)
    local object = {
        speed = 40,
        drawer = {},
        life_time = 0.25,
        current_time = 0,
        aabb = {},
        texture = {},
        setup = function(self, owner)
            self.aabb = aabb()
            self.texture = texture()
            self.texture:fill_color(color(1.0, 1.0, 1.0, 1.0))
            self.drawer = draw3d(self.texture)
            self.drawer.vertex_name = "BOX"
            self.drawer.position = vector3(owner.position.x, owner.position.y,
                owner.position.z)
            self.drawer.rotation = owner.rotation
            self.drawer.scale = vector3(0.2, 0.2, 0.2)
            sound:play()
        end,
        update = function(self)
            self.aabb.max = self.drawer.position:add(
                self.drawer.scale:mul(m.aabb.max))
            self.aabb.min = self.drawer.position:add(
                self.drawer.scale:mul(m.aabb.min))
            self.current_time = self.current_time + delta_time
            self.drawer.position.x = self.drawer.position.x + delta_time *
                self.speed *
                math.cos(
                    math.rad(self.drawer.rotation.z))
            self.drawer.position.y = self.drawer.position.y + delta_time *
                self.speed *
                math.sin(
                    math.rad(self.drawer.rotation.z))
        end,
        draw = function(self)
            self.drawer:draw()
        end
    }

    return object
end

return bullet
