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
    hp_drawer2 = {},
    hp_font = {},
    hp_font_texture = {},
    hp_font_texture2 = {},
    aabb = {},
    bullet_time = {},
    bullet_timer = {},
    efks = {},
    tex_scope = {},
    drawer_scope = {},
    drawer_scope_prev = vector2(0, 0),
    is_shot = true,
    setup = function(self, map, map_size_x, map_size_y)
        self.model = model()
        self.model:load("triangle.sim", "player")
        self.drawer = draw3d(tex)
        self.drawer.vertex_name = "player"
        self.aabb = aabb()
        self.bullet_time = 0.1
        self.bullet_timer = 0.0
        self.hp = 100
        self.hp_font_texture = texture()
        self.hp_font_texture2 = texture()
        self.hp_drawer = draw2d(self.hp_font_texture)
        self.hp_drawer2 = draw2d(self.hp_font_texture2)
        self.hp_font_texture2:fill_color(color(0.2, 0.2, 0.2, 0.2))
        self.render_text(self)
        r1 = 0
        r2 = 0
        while decide_pos(map, map_size_x, map_size_y) == true do end
        self.drawer.position = vector3(r1 * 2, r2 * 2, 0)
        self.drawer.scale = vector3(0.7, 0.7, 0.7)
        self.hp_drawer.position.x = 0
        self.hp_drawer.position.y = 300
        self.hp_drawer2.position.x = 0
        self.hp_drawer2.position.y = 300
        self.hp_drawer2.scale = vector2(self.hp * 10.0, 50)
        self.tex_scope = texture()
        self.tex_scope:load("scope.png")
        self.drawer_scope = draw2d(self.tex_scope)
        self.drawer_scope.scale = self.tex_scope:size()

    end,
    horizontal = math.pi,
    vertical = 0.0,
    update = function(self, map, map_draw3ds, map_size_x, map_size_y)
        self.drawer.rotation.z = math.deg(math.atan(self.drawer_scope.position.y,
            self.drawer_scope.position.x)) - 90
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
            self.bullet_time and (keyboard:is_key_down(keySPACE) or mouse:is_button_down(mouseLEFT)) then
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
        scale = self.drawer.scale.x * 2.0
        before_pos = vector3(self.drawer.position.x, self.drawer.position.y,
            self.drawer.position.z)
        if input_vector.y ~= 0 then
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position:add(vector3(0,
                input_vector.y *
                scale *
                speed *
                delta_time,
                0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        if input_vector.x ~= 0 then
            self.drawer.position = self.drawer.position:add(vector3(
                input_vector.x *
                scale *
                speed *
                delta_time,
                0, 0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
            before_pos = self.drawer.position:copy()
            self.drawer.position = self.drawer.position:add(vector3(0,
                input_vector.y *
                scale *
                speed *
                delta_time,
                0))
            if is_collision(self, map, map_draw3ds, map_size_x, map_size_y) then
                self.drawer.position = before_pos
            end
        end
        local mouse_pos = mouse:position()
        if mouse_pos.x == self.drawer_scope_prev.x and mouse_pos.y ==
            self.drawer_scope_prev.y then
            local speed = 1000 * delta_time
            if keyboard:is_key_down(keyUP) then
                self.drawer_scope.position.y = self.drawer_scope.position.y + speed
            end
            if keyboard:is_key_down(keyDOWN) then
                self.drawer_scope.position.y = self.drawer_scope.position.y - speed
            end
            if keyboard:is_key_down(keyLEFT) then
                self.drawer_scope.position.x = self.drawer_scope.position.x - speed
            end
            if keyboard:is_key_down(keyRIGHT) then
                self.drawer_scope.position.x = self.drawer_scope.position.x + speed
            end
        else
            self.drawer_scope.position.x = mouse_pos.x - window.size().x / 2
            self.drawer_scope.position.y = -(mouse_pos.y - window.size().y / 2)
        end
        self.drawer_scope_prev = vector2(mouse_pos.x, mouse_pos.y)
        mouse:hide_cursor(true)

    end,

    draw = function(self)
        if not fps_mode then self.drawer:draw() end
        self.hp_drawer:draw()
        self.hp_drawer2:draw()
        self.drawer_scope:draw()
    end,
    render_text = function(self)
        if self.hp < 20 then
            self.hp_font_texture:fill_color(color(1, 0.6, 0.6, 0.8))
        else
            self.hp_font_texture:fill_color(color(0.6, 1, 0.6, 0.8))
        end
        self.hp_drawer.scale = vector2(self.hp * 10, 50)
    end
}

return player
