local m = Model()
local sound = Sound()
sound:load("shot.wav")
sound:set_volume(0.3)

local function bullet(map_draw3ds)
    local object = {
        speed = 40,
        drawer = {},
        life_time = 0.25,
        current_time = 0,
        aabb = {},
        texture = {},
        setup = function(self, owner)
            self.aabb = AABB()
            self.texture = Texture()
            self.texture:fill_color(Color(1.0, 1.0, 1.0, 1.0))
            self.drawer = Draw3D(self.texture)
            self.drawer.position = Vector3(owner.position.x, owner.position.y,
                owner.position.z)
            self.drawer.rotation = owner.rotation
            self.drawer.scale = Vector3(0.2, 0.2, 0.2)
            sound:play()
        end,
        update = function(self)
            self.aabb.max = self.drawer.position + (
                self.drawer.scale * m:aabb().max)
            self.aabb.min = self.drawer.position + (
                self.drawer.scale * m:aabb().min)
            self.current_time = self.current_time + scene.delta_time()
            self.drawer.position.x = self.drawer.position.x + scene.delta_time() *
                self.speed *
                math.cos(
                    math.rad(self.drawer.rotation.z))
            self.drawer.position.y = self.drawer.position.y + scene.delta_time() *
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
