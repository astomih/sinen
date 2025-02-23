local bullet = require "bullet"
local calc_input_vector = require "calc_input_vector"
local is_collision = require "is_collision"
local input_vector = {}
local speed = 2.0
local effect = require "effect"
local orbit = require "orbit"
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
    hp_max = 100,
    hp_drawer = {},
    hp_font = {},
    hp_font_texture = {},
    stamina = {},
    stamina_max = 100,
    stamina_recover_speed = 20,
    stamina_boost_cost = 20,
    stamina_run_cost = 5,
    stamina_texture = {},
    stamina_drawer = {},
    stamina_max_texture = {},
    stamina_max_drawer = {},
    oil = {},
    oil_max = 100,
    oil_texture = {},
    oil_drawer = {},
    oil_max_texture = {},
    oil_max_drawer = {},
    aabb = {},
    bullet_time = {},
    bullet_timer = {},
    bullet_flag = false,
    efks = {},
    tex_scope = {},
    drawer_scope = {},
    big_scope = {},
    drawer_scope_big = {},
    is_shot = true,
    boost = 0.0,
    boost_time = 0.3,
    boost_timer = 0.0,
    boost_mag = 5,
    boost_sound = {},
    is_boost = false,
    orbits = {},
    orbit_toggle = true,
    speed_min = 6.0,
    speed_max = 16.0,
    blur_time = 0.0,
    boost_reset = function(self)
        self.boost_timer = 0.0
        self.boost = 0.0
        self.is_boost = false
        renderer.at_render_texture_user_data(0, 0.0)
    end,
    setup = function(self, map, map_size_x, map_size_y)
        self.model = model()
        self.model:load("triangle.sim")
        self.drawer = draw3d(DEFAULT_TEXTURE)
        self.drawer.model = self.model
        self.aabb = aabb()
        self.bullet_time = 0.1
        self.bullet_timer = 0.0
        self.hp = 100
        self.hp_font = font()
        self.hp_font:load(DEFAULT_FONT_NAME, 64)
        self.hp_font_texture = texture()
        self.hp_drawer = draw2d(self.hp_font_texture)
        self.stamina = self.stamina_max
        self.stamina_texture = texture()
        self.stamina_texture:fill_color(color(1.0, 1.0, 1.0, 0.9))
        self.stamina_max_texture = texture()
        self.stamina_max_texture:fill_color(color(0.0, 0.0, 0.0, 0.2))
        self.stamina_drawer = draw2d(self.stamina_texture)
        self.stamina_drawer.position = vector2(0, 350)
        self.stamina_drawer.scale = UI_SCALE_VECTOR2(300, 10)
        self.stamina_max_drawer = draw2d(self.stamina_max_texture)
        self.stamina_max_drawer.position = vector2(0, 350)
        self.stamina_max_drawer.scale = UI_SCALE_VECTOR2(300, 10)

        self.oil = self.oil_max
        self.oil_texture = texture()
        self.oil_texture:fill_color(color(1.0, 1.0, 1.0, 0.9))
        self.oil_max_texture = texture()
        self.oil_max_texture:fill_color(color(0.0, 0.0, 0.0, 0.2))
        self.oil_drawer = draw2d(self.oil_texture)
        -- oil drawer position is left bottom
        self.oil_drawer.position = vector2(-scene.center().x + 20, -scene.center().y + 150)
        self.oil_drawer.scale = vector2(10, 300)
        self.oil_max_drawer = draw2d(self.oil_max_texture)
        self.oil_max_drawer.position = vector2(-scene.center().x + 20, -scene.center().y + 150)
        self.oil_max_drawer.scale = vector2(10, 300)



        self.render_text(self)
        r1 = 0
        r2 = 0
        while decide_pos(map, map_size_x, map_size_y) == true do
        end
        self.drawer.position = vector3(r1 * 2, r2 * 2, 1)
        self.drawer.scale = vector3(0.05, 0.05, 0.1)
        self.hp_drawer.position.x = 0
        self.hp_drawer.position.y = 300
        self.tex_scope = texture()
        self.tex_scope:load("scope.png")
        self.big_scope = texture()
        self.big_scope:load("scope_big.png")
        self.drawer_scope = draw2d(self.tex_scope)
        self.drawer_scope_big = draw2d(self.big_scope)
        self.drawer_scope.scale = self.tex_scope:size()
        self.drawer_scope_big.scale = vector2(self.big_scope:size().x * 2, self.big_scope:size().y * 2)
        self.boost_sound = sound()
        self.boost_sound:load("boost.wav")
        self.boost_sound:set_volume(0.2)
    end,
    horizontal = math.pi,
    vertical = 0.0,
    update = function(self, map, map_draw3ds, map_size_x, map_size_y)
        local p = self.drawer.position:copy()
        self.aabb:update_world(self.drawer.position, self.drawer.scale, self.model:aabb())
        input_vector = calc_input_vector()
        local is_move = input_vector.x ~= 0 or input_vector.y ~= 0

        if keyboard.is_down(keyboard.LSHIFT) and is_move then
            speed = self.speed_max
            self.stamina = self.stamina - self.stamina_run_cost * scene.delta_time()
            if self.stamina <= 0.0 then
                self.stamina = 0.0
                speed = self.speed_min
            end
        else
            speed = self.speed_min
            self.stamina = self.stamina + scene.delta_time() * self.stamina_recover_speed
            if self.stamina > self.stamina_max then
                self.stamina = self.stamina_max
            end
        end
        -- bullet
        if keyboard.is_pressed(keyboard.Q) then
            if self.orbit_toggle then
                self.orbit_toggle = false
                local o = orbit(self)
                o:setup()
                table.insert(self.orbits, o)
            else
                self.orbit_toggle = true
                self.orbits = {}
            end
        end
        if mouse.is_pressed(mouse.LEFT) then
            self.bullet_flag = true
        end
        self.bullet_timer = self.bullet_timer + scene.delta_time()
        if self.bullet_flag then
            if self.bullet_timer >
                self.bullet_time and (mouse.is_down(mouse.LEFT)) then
                local b = bullet(map_draw3ds)
                b:setup(self.drawer)
                b.drawer.rotation.z = b.drawer.rotation.z + 90
                table.insert(self.bullets, b)
                self.bullet_timer = 0.0
                self.oil = self.oil - 1
            end
            if mouse.is_released(mouse.LEFT) then
                self.bullet_flag = false
            end
        end
        if self.is_boost then
            if self.boost_timer >= self.boost_time then
                self:boost_reset()
            else
                local t = periodic.sin0_1(self.boost_time * 2.0, self.boost_timer) - 0.5
                t = t * 0.2

                renderer.at_render_texture_user_data(0, t)
                self.boost_timer = self.boost_timer + scene.delta_time()
            end
        else
            if keyboard.is_pressed(keyboard.SPACE) and is_move then
                if self.stamina >= self.stamina_boost_cost then
                    self.stamina = self.stamina - self.stamina_boost_cost
                    if self.stamina <= 0.0 then
                        self.stamina = 0.0
                    end
                    local efk = effect()
                    efk:setup()
                    efk.texture:fill_color(color(0.6, 0.6, 1.0, 1.0))
                    efk.impl = function(e)
                        for i = 1, e.max_particles do
                            local t = scene.delta_time() * 2
                            e.worlds[i].position.x =
                                e.worlds[i].position.x + math.cos(i) * t
                            e.worlds[i].position.y =
                                e.worlds[i].position.y + math.sin(i) * t
                            e.worlds[i].position.z =
                                e.worlds[i].position.z + t
                        end
                    end
                    for j = 1, efk.max_particles do
                        efk.worlds[j].position = self.drawer.position:copy()
                    end
                    efk:play()
                    table.insert(self.efks, efk)
                    self.boost_sound:play()
                    self.boost = self.boost_mag
                    self.is_boost = true
                    self.boost_timer = 0.0
                end
            end
        end
        for i, v in ipairs(self.bullets) do
            v:update()
            if v.current_time > v.life_time then
                local efk = effect()
                efk:setup()
                efk.texture:fill_color(color(1.0, 1.0, 1.0, 1.0))
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
        local before_pos = vector3(self.drawer.position.x, self.drawer.position.y,
            self.drawer.position.z)
        local final_speed = 0.0
        if self.is_boost then
            final_speed = self.speed_max * self.boost
        else
            final_speed = speed
        end
        if input_vector.x ~= 0 and input_vector.y ~= 0 then
            final_speed = final_speed / math.sqrt(2)
        end
        if input_vector.y ~= 0 then
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position + vector3(0,
                input_vector.y *
                final_speed *
                scene.delta_time(),
                0)
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        if input_vector.x ~= 0 then
            self.drawer.position = self.drawer.position + vector3(
                input_vector.x *
                final_speed *
                scene.delta_time(),
                0, 0)
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position + vector3(0,
                input_vector.y *
                final_speed *
                scene.delta_time(),
                0)
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        local mouse_pos = mouse.position_on_scene()
        self.drawer_scope.position.x = mouse_pos.x - scene.center().x
        self.drawer_scope.position.y = -(mouse_pos.y - scene.center().y)
        local r = 200 - self.drawer_scope.scale.x / 2
        -- Make the coordinates fit in a circle of radius r
        local x = self.drawer_scope.position.x
        local y = self.drawer_scope.position.y
        local d = self.drawer_scope.position:length()
        local ratio = scene.ratio()
        self.drawer_scope_big.scale = vector2(self.big_scope:size().x * 2 * ratio.x, self.big_scope:size().y * 2 *
            ratio.y)
        if d > r then
            local r_prime = r / d - 0.01
            x = x * r_prime
            y = y * r_prime
            self.drawer_scope.position.x = x
            self.drawer_scope.position.y = y
            mouse.set_position_on_scene(vector2(scene.center().x + x, scene.center().y - y))
        end
        local drawer_scope_angle = math.atan(self.drawer_scope.position.y, self.drawer_scope.position.x)
        self.drawer.rotation.z = math.deg(drawer_scope_angle) - 90
        for i, j in ipairs(self.orbits) do
            j:update(map_draw3ds)
        end
        local s_ratio = self.stamina / self.stamina_max
        self.stamina_drawer.scale.x = s_ratio * 300
        if s_ratio <= 0.2 then
            self.stamina_texture:fill_color(color(1.0, 0.0, 0.0, 0.9))
        else
            self.stamina_texture:fill_color(color(1.0, 1.0, 1.0, 0.9))
        end
        local o_ratio = self.oil / self.oil_max
        self.oil_drawer.scale.y = o_ratio * 300
        if o_ratio <= 0.2 then
            self.oil_texture:fill_color(color(1.0, 0.0, 0.0, 0.9))
        else
            self.oil_texture:fill_color(color(1.0, 1.0, 1.0, 0.9))
        end
    end,
    draw = function(self)
        self.drawer:draw()
        self.hp_drawer:draw()
        self.stamina_max_drawer:draw()
        self.stamina_drawer:draw()
        self.oil_max_drawer:draw()
        self.oil_drawer:draw()
        self.drawer_scope_big:draw()
        self.drawer_scope:draw()
        for i, v in ipairs(self.efks) do
            v:draw()
        end
        for i, v in ipairs(self.bullets) do
            v:draw()
        end
        for i, j in ipairs(self.orbits) do
            j:draw()
        end
    end,
    render_text = function(self)
        if self.hp / self.hp_max <= 0.2 then
            self.hp_font:render_text(self.hp_font_texture, "HP: " .. self.hp, color(1, 0.0, 0.0, 0.8))
        else
            self.hp_font:render_text(self.hp_font_texture, "HP: " .. self.hp, color(1, 1, 1, 0.9))
        end
        self.hp_drawer.scale = self.hp_font_texture:size()
    end
}

return player
