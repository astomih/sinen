local bullet = require "bullet"
local calc_input_vector = require "calc_input_vector"
local is_collision = require "is_collision"
local input_vector = {}
local speed = 2.0
local effect = require "effect"
local r1 = 0
local r2 = 0
local function decide_pos(map, map_size_x, map_size_y)
    r1 = math.random(1, map_size_x)
    r2 = math.random(1, map_size_y)
    return map:at(r1, r2) == 1
end

local player = {
    drawer = {},
    model = {},
    bullets = {},
    hp = {},
    hp_drawer = {},
    hp_font = {},
    hp_font_texture = {},
    aabb = {},
    bullet_time = {},
    bullet_timer = {},
    efk = {},
    efks = {},
    tex_scope = {},
    drawer_scope = {},
    drawer_scope_big = {},
    is_shot = true,
    boost = 1.0,
    boost_time = 0.3,
    boost_timer = 0.0,
    boost_mag = 5,
    is_boost = false,
    setup = function(self, map, map_size_x, map_size_y)
        self.model = model()
        self.model:load("triangle.sim", "player")
        self.drawer = draw3d(tex)
        self.drawer.vertex_name = "player"
        self.aabb = aabb()
        self.bullet_time = 0.1
        self.bullet_timer = 0.0
        self.hp = 100
        self.hp_font = font()
        self.hp_font:load("x16y32pxGridGazer.ttf", 64)
        self.hp_font_texture = texture()
        self.hp_drawer = draw2d(self.hp_font_texture)
        self.render_text(self)
        r1 = 0
        r2 = 0
        while decide_pos(map, map_size_x, map_size_y) == true do end
        self.drawer.position = vector3(r1 * 2, r2 * 2, 0)
        self.drawer.scale = vector3(0.06, 0.06, 0.06)
        self.hp_drawer.position.x = 0
        self.hp_drawer.position.y = 300
        self.tex_scope = texture()
        self.tex_scope:load("scope.png")
        local big_scope = texture()
        big_scope:load("scope_big.png")
        self.drawer_scope = draw2d(self.tex_scope)
        self.drawer_scope_big = draw2d(big_scope)
        self.drawer_scope.scale = self.tex_scope:size()
        self.drawer_scope_big.scale = vector2(big_scope:size().x * 2, big_scope:size().y * 2)
        self.efk = effect()
        self.efk.start_lifetime = 0.3
        self.efk:setup()
        local p = self.drawer.position:copy()
        for j = 1, self.efk.max_particles do
            self.efk.worlds[j].position = p
        end
        self.efk:play()


    end,
    horizontal = math.pi,
    vertical = 0.0,
    update = function(self, map, map_draw3ds, map_size_x, map_size_y)
        self.efk:update()
        local p = self.drawer.position:copy()
        if self.efk.is_stop then
            self.efk:setup()
            for j = 1, self.efk.max_particles do
                self.efk.worlds[j].position = self.drawer.position:copy()
            end
            self.efk:play()
        end
        if keyboard:key_state(keyV) == buttonPRESSED then
            self.drawer.rotation.z = self.drawer.rotation.z + 180
        end
        self.aabb.max = self.drawer.position:add(
            self.drawer.scale:mul(self.model.aabb.max))
        self.aabb.min = self.drawer.position:add(
            self.drawer.scale:mul(self.model.aabb.min))
        if self.hp <= 0 then change_scene("scene03_gameover") end
        input_vector = calc_input_vector()
        if keyboard:is_key_down(keyLSHIFT) then
            speed = 8.0
        else
            speed = 4.0
        end
        if input_vector.x ~= 0 and input_vector.y ~= 0 then
            speed = speed / math.sqrt(2)
        end
        -- bullet
        if keyboard:key_state(keyE) == buttonPRESSED then
            self.is_shot = not self.is_shot
        end
        self.bullet_timer = self.bullet_timer + delta_time
        if self.bullet_timer >
            self.bullet_time and (mouse:is_button_down(mouseLEFT)) then
            local b = bullet(map_draw3ds)
            b:setup(self.drawer)
            b.drawer.rotation.z = b.drawer.rotation.z + 90
            table.insert(self.bullets, b)
            self.bullet_timer = 0.0
        end
        if self.is_boost then
            if self.boost_timer >= self.boost_time then
                self.boost_timer = 0.0
                self.boost = self.boost - self.boost_mag
                self.is_boost = false
            else
                self.boost_timer = self.boost_timer + delta_time
            end
        else
            if keyboard:key_state(keySPACE) == buttonPRESSED then
                self.boost = self.boost + self.boost_mag
                self.is_boost = true
                self.boost_timer = 0.0
            end
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
        scale = self.drawer.scale.x * 2.0
        before_pos = vector3(self.drawer.position.x, self.drawer.position.y,
            self.drawer.position.z)
        if input_vector.y ~= 0 then
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position:add(vector3(0,
                input_vector.y *
                speed * self.boost *
                delta_time,
                0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        if input_vector.x ~= 0 then
            self.drawer.position = self.drawer.position:add(vector3(
                input_vector.x *
                speed * self.boost *
                delta_time,
                0, 0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position:add(vector3(0,
                input_vector.y *
                speed * self.boost *
                delta_time,
                0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        local mouse_pos = mouse:position()
        local speed = 1000 * delta_time
        local pressed = false
        if keyboard:is_key_down(keyUP) then
            self.drawer_scope.position.y = self.drawer_scope.position.y + speed
            pressed = true
        end
        if keyboard:is_key_down(keyDOWN) then
            self.drawer_scope.position.y = self.drawer_scope.position.y - speed
            pressed = true
        end
        if keyboard:is_key_down(keyLEFT) then
            self.drawer_scope.position.x = self.drawer_scope.position.x - speed
            pressed = true
        end
        if keyboard:is_key_down(keyRIGHT) then
            self.drawer_scope.position.x = self.drawer_scope.position.x + speed
            pressed = true
        end
        if not pressed then
            self.drawer_scope.position.x = mouse_pos.x - window.size().x / 2
            self.drawer_scope.position.y = -(mouse_pos.y - window.size().y / 2)
        end
        local r = 200
        -- Make the coordinates fit in a circle of radius r
        local x = self.drawer_scope.position.x
        local y = self.drawer_scope.position.y
        local d = self.drawer_scope.position:length()
        if d > r then
            x = x * r / d
            y = y * r / d
            self.drawer_scope.position.x = x
            self.drawer_scope.position.y = y
            mouse:set_position(vector2(window.size().x / 2 + x, window.size().y / 2 - y))
        end
        local drawer_scope_angle = math.atan(self.drawer_scope.position.y, self.drawer_scope.position.x)
        self.drawer.rotation.z = math.deg(drawer_scope_angle) - 90
        mouse:hide_cursor(true)

    end,

    draw = function(self)
        if not fps_mode then self.drawer:draw() end
        self.hp_drawer:draw()
        self.drawer_scope_big:draw()
        self.drawer_scope:draw()
    end,
    render_text = function(self)
        self.hp_font:render_text(self.hp_font_texture, "HP: " .. self.hp, color(1, 1, 1, 1))
        self.hp_drawer.scale = self.hp_font_texture:size()
    end
}

return player
