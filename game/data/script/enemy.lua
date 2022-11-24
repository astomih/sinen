local is_collision = require "is_collision"

local function same(t, p, comp)
    for k, v in ipairs(t) do if not comp(v, p[k]) then return false end end
    return true
end

local comp = function(a, b) return a.x == b[1] and a.y == b[2] end

local bombed = sound()
bombed:load("bombed.wav")
local r1 = 0
local r2 = 0
local function decide_pos(map, map_size_x, map_size_y)
    r1 = math.random(1, map_size_x)
    r2 = math.random(1, map_size_y)
    return map:at(r1, r2) == 1
end

local enemy = function()
    local object = {
        drawer = {},
        speed = 4,
        model = {},
        hp = 100,
        aabb = {},
        is_collision_first = {},
        collision_time = {},
        collision_timer = {},
        map = {},
        get_forward_z = function(drawer)
            return vector2(-math.sin(math.rad(drawer.rotation.z)),
                math.cos(math.rad(-drawer.rotation.z)))
        end,
        bfs = {},
        setup = function(self, _map, map_size_x, map_size_y)
            self.bfs = bfs_grid(_map)
            self.drawer = draw3d(tex)
            self.model = model()
            if now_stage == 1 then
                self.model:load("spider.sim", "spider")
                self.drawer.vertex_name = "spider"
                self.drawer.scale = vector3(0.3, 0.3, 0.3)
            end
            if now_stage == 2 then
                if math.random(0, 1) == 0 then
                    self.model:load("bat.sim", "bat")
                    self.drawer.vertex_name = "bat"
                    self.drawer.scale = vector3(0.4, 0.4, 0.4)
                else
                    self.model:load("lizard.sim", "lizard")
                    self.drawer.vertex_name = "lizard"
                    self.drawer.scale = vector3(1, 1, 1)
                end
            end
            if now_stage == 3 then
                self.model:load("frog.sim", "frog")
                self.drawer.vertex_name = "frog"
                self.drawer.scale = vector3(1, 1, 1)
            end
            self.aabb = aabb()
            self.map = _map
            r1 = 0
            r2 = 0
            while decide_pos(_map, map_size_x, map_size_y) == true do end
            self.drawer.position = vector3(r1 * 2, r2 * 2, 1)
            self.is_collision_first = true
            self.collision_time = 1.0
            self.collision_timer = 0.0
        end,
        update = function(self, player, map, map_draw3ds, map_size_x, map_size_y)
            self.aabb.max = self.drawer.position:add(
                self.drawer.scale:mul(self.model.aabb.max))
            self.aabb.min = self.drawer.position:add(
                self.drawer.scale:mul(self.model.aabb.min))
            self.drawer.rotation = vector3(0, 0,
                math.deg(
                    -math.atan(
                        player.drawer.position.x -
                        self.drawer.position.x,
                        player.drawer.position.y -
                        self.drawer.position.y)))
            local start = point2i(math.floor(
                self.drawer.position.x / 2 +
                0.5), math.floor(
                self.drawer.position.y / 2 +
                0.5))
            local goal = point2i(math.floor(
                player.drawer.position.x / 2 +
                0.5), math.floor(
                player.drawer.position.y / 2 +
                0.5))
            self.bfs:find_path(start, goal)
            if self.bfs:traceable() then
                local path = self.bfs:trace()
                path = self.bfs:trace()
                self.drawer.position.x =
                self.drawer.position.x +
                    (path.x * 2 - self.drawer.position.x) * delta_time *
                    3.0
                self.drawer.position.y =
                self.drawer.position.y +
                    (path.y * 2 - self.drawer.position.y) * delta_time *
                    3.0
            else
                self.drawer.position.x =
                self.drawer.position.x + delta_time * self.speed *
                    self.get_forward_z(self.drawer).x
                self.drawer.position.y =
                self.drawer.position.y + delta_time * self.speed *
                    self.get_forward_z(self.drawer).y
            end
            self.bfs:reset()

        end,
        draw = function(self) self.drawer:draw() end,
        player_collision = function(self, player)
            if self.aabb:intersects_aabb(player.aabb) then
                if self.is_collision_first then
                    bombed:play()
                    player.hp = player.hp - 1
                    player:render_text()
                    self.is_collision_first = false
                else
                    self.collision_timer = self.collision_timer + delta_time
                    if self.collision_timer > self.collision_time then
                        bombed:play()
                        player.hp = player.hp - 10
                        player:render_text()
                        self.collision_timer = 0.0
                    end
                end
            else
                self.is_collision_first = true
            end
        end
    }
    return object
end
return enemy
