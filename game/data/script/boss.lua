local function xor(a, b) return (a and not b) or (not a and b) end

local boss = {
    drawer = {},
    model = {},
    setup = function(self)

        self.model = model()
        self.model:load("boss.sim", "boss")
        self.drawer = draw3d(tex)
        self.drawer.vertex_name = "boss"
    end,
    update = function(self) end,
    draw = function(self) self.drawer:draw() end
}

return boss
