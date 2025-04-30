local world = require "world"
local tree = Model()

local function effect()
    local object = {
        duration = 1.0,
        looping = false,
        prewarm = false,
        start_lifetime = 1.0,
        start_speed = 5.0,
        start_size = 1.0,
        start_rotation = 0.0,
        start_color = Color(1, 1, 1, 1),
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
            self.texture = Texture()
            self.texture:fill_color(self.start_color)
            self.drawer = Draw3D(self.texture)
            for i = 1, self.max_particles do
                self.worlds[i] = world()
                self.worlds[i].position = Vec3(0, 0, 0)
                self.worlds[i].rotation = Vec3(0, 0, 0)
                self.worlds[i].scale = Vec3(0.1, 0.1, 0.1)
            end
            if self.play_on_awake then self.is_playing = true end
        end,
        impl = function(e)
            for i = 1, e.max_particles do
                e.worlds[i].position.x =
                    e.worlds[i].position.x + math.cos(i) * scene.delta_time()
                e.worlds[i].position.y =
                    e.worlds[i].position.y + math.sin(i) * scene.delta_time()
                e.worlds[i].position.z =
                    e.worlds[i].position.z + scene.delta_time()
            end
        end,
        update = function(self)
            --  if not self.is_playing then return end
            self.drawer:clear()
            self.timer = self.timer + scene.delta_time()
            if self.timer > self.start_lifetime then
                self.timer = 0.0
                for i = 1, self.max_particles do
                    self.worlds[i].position = Vec3(0, 0, 0)
                    self.worlds[i].rotation = Vec3(0, 0, 0)
                    self.worlds[i].scale = Vec3(1, 1, 1)
                end
                if not self.looping then
                    self.is_playing = false
                    self.is_stop = true
                end
            end
            self.impl(self)
            if self.is_playing then
                for i = 1, self.max_particles do
                    self.drawer:add(self.worlds[i].position,
                        self.worlds[i].rotation,
                        self.worlds[i].scale)
                end
            end
        end,
        draw = function(self)
            if self.is_playing then
                self.drawer:draw()
            end
        end,
        play = function(self)
            self.is_playing = true
            self.is_stop = false
        end
    }
    return object
end

return effect
