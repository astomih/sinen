local shadow_tex = texture()
shadow_tex:load("shadow.png")
local function shadow()
    local object = {
        owner = {},
        drawer = {},
        setup = function(self)
            self.drawer = draw3d(shadow_tex)
            self.drawer.position = self.owner.position
            self.drawer.scale = vector3(2, 2, 0)
            self.drawer.position.z = 1
        end,

        update = function(self)
            self.drawer.position = self.owner.position
            self.drawer.position.z = 0.1
        end,
        draw = function(self) -- self.drawer:draw() 
        end
    }
    return object
end

return shadow
