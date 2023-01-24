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
        search_length = 15,
        model = {},
        hp = 30,
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
            self.drawer = draw3d(DEFAULT_TEXTURE)
            self.model = model()
            if NOW_STAGE == 1 then
                self.model:load("spider.sim", "spider")
                self.drawer.vertex_name = "spider"
                self.drawer.scale = vector3(0.3, 0.3, 0.3)
            end
            if NOW_STAGE == 2 then
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
            if NOW_STAGE == 3 then
                self.model:load("frog.sim", "frog")
                self.drawer.vertex_name = "frog"
                self.drawer.scale = vector3(1, 1, 1)
            end
            self.aabb = aabb()
            self.map = _map
            r1 = 0
            r2 = 0
            while decide_pos(_map, map_size_x, map_size_y) == true do end
            self.drawer.position = vector3(r1 * TILE_SIZE, r2 * TILE_SIZE, 0.5)
            self.is_collision_first = true
            self.collision_time = 1.0
            self.collision_timer = 0.0
        end,
        update = function(self, player)
            local length = self.drawer.position:sub(player.drawer.position):length()
            if length > self.search_length then
                return
            end
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
            local start = point2i(
                self.drawer.position.x / TILE_SIZE
                ,
                self.drawer.position.y / TILE_SIZE
            )
            local goal = point2i(
                player.drawer.position.x / TILE_SIZE,
                player.drawer.position.y / TILE_SIZE
            )
            if self.bfs:find_path(start, goal) then
                local path = self.bfs:trace()
                path = self.bfs:trace()
                self.drawer.position.x =
                self.drawer.position.x +
                    (path.x * TILE_SIZE - self.drawer.position.x) * delta_time * self.speed
                self.drawer.position.y =
                self.drawer.position.y +
                    (path.y * TILE_SIZE - self.drawer.position.y) * delta_time * self.speed

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
            if collision.aabb_aabb(self.aabb, player.aabb) then
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
