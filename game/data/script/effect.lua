local world = require "world"
local tree = model()
tree:load("tree.sim", "tree")

local function effect()
    local object = {
        duration = 1.0,
        looping = false,
        prewarm = false,
        start_lifetime = 0.5,
        start_speed = 5.0,
        start_size = 1.0,
        start_rotation = 0.0,
        start_color = color(1, 1, 1, 1),
        gravity_multiplier = 0.0,
        inherit_velocity = 0.0,
        play_on_awake = false,
        max_particles = 10,

        drawer = {},
        texture = {},
        worlds = {},

        is_playing = false,
        is_stop = false,

        timer = 0.0,
        setup = function(self)
            self.texture = texture()
            self.texture:fill_color(self.start_color)
            self.drawer = draw3d_instanced(self.texture)
            self.drawer.vertex_name = "BOX"
            for i = 1, self.max_particles do
                self.worlds[i] = world()
                self.worlds[i].position = vector3(0, 0, 0)
                self.worlds[i].rotation = vector3(0, 0, 0)
                self.worlds[i].scale = vector3(0.1, 0.1, 0.1)
            end
            if self.play_on_awake then self.is_playing = true end

        end,

        update = function(self)
            --  if not self.is_playing then return end
            self.texture:fill_color(color(1, 0, 0, 1))
            self.drawer:clear()
            self.timer = self.timer + delta_time
            if self.timer > self.start_lifetime then
                self.timer = 0.0
                for i = 1, self.max_particles do
                    self.worlds[i].position = vector3(0, 0, 0)
                    self.worlds[i].rotation = vector3(0, 0, 0)
                    self.worlds[i].scale = vector3(1, 1, 1)
                end
                if not self.looping then
                    self.is_playing = false
                    self.is_stop = true
                end
            end
            for i = 1, self.max_particles do
                self.worlds[i].position.x =
                self.worlds[i].position.x + math.cos(i) * delta_time
                self.worlds[i].position.y =
                self.worlds[i].position.y + math.sin(i) * delta_time
                self.worlds[i].position.z =
                self.worlds[i].position.z + delta_time
            end
            if self.is_playing then
                for i = 1, self.max_particles do
                    self.drawer:add(self.worlds[i].position,
                        self.worlds[i].rotation,
                        self.worlds[i].scale)

                end
                self.drawer:draw()
            end
        end,
        play = function(self) self.is_playing = true end

    }
    return object
end

return effect
