local bombed = sound()
bombed:load("bombed.wav")
local r1 = 0
local r2 = 0
local function decide_pos(map, map_size_x, map_size_y)
    r1 = math.random(1, map_size_x)
    r2 = math.random(1, map_size_y)
    return map:at(r1, r2) < MAP_CHIP_WALKABLE
end

local enemy_model = {}
enemy_model[1] = model()
enemy_model[1]:load("enemy1.sim", "spider")
enemy_model[2] = model()
enemy_model[2]:load("bat.sim", "bat")
enemy_model[3] = model()
enemy_model[3]:load("lizard.sim", "lizard")
enemy_model[4] = model()
enemy_model[4]:load("frog.sim", "frog")
for i = 1, 4 do
    enemy_model[i].aabb.max.z = 10.0
    enemy_model[i].aabb.min.z = -10.0
end


local enemy = function()
    local object = {
        drawer = {},
        speed = 10,
        search_length = 15,
        hp = 30,
        aabb = {},
        is_collision_first = {},
        collision_time = {},
        collision_timer = {},
        map = {},
        model_index = 1,
        get_forward_z = function(drawer)
            return vector2(-math.sin(math.rad(drawer.rotation.z)),
                math.cos(math.rad(-drawer.rotation.z)))
        end,
        bfs = {},
        -- Add damage to enemy
        -- @param damage Damage value
        -- @return true if enemy is dead
        -- @return false if enemy is alive
        add_damage = function(self, damage)
            self.hp = self.hp - damage
            if self.hp <= 0 then
                return true
            end
            return false
        end,
        setup = function(self, _map, map_size_x, map_size_y)
            self.bfs = bfs_grid(_map)
            self.drawer = draw3d(DEFAULT_TEXTURE)
            if NOW_STAGE == 1 then
                self.drawer.vertex_name = "spider"
                self.drawer.scale = vector3(0.1, 0.1, 0.1)
                self.model_index = 1
            end
            if NOW_STAGE == 2 then
                if math.random(0, 1) == 0 then
                    self.drawer.vertex_name = "bat"
                    self.drawer.scale = vector3(0.4, 0.4, 0.4)
                    self.model_index = 2
                else
                    self.drawer.vertex_name = "lizard"
                    self.drawer.scale = vector3(1, 1, 1)
                    self.model_index = 3
                end
            end
            if NOW_STAGE == 3 then
                self.drawer.vertex_name = "frog"
                self.drawer.scale = vector3(1, 1, 1)
                self.model_index = 4
            end
            self.aabb = aabb()
            self.map = _map
            r1 = 0
            r2 = 0
            while decide_pos(_map, map_size_x, map_size_y) == true do
            end
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
            self.aabb:update_world(self.drawer.position, self.drawer.scale, enemy_model[self.model_index].aabb)
            -- If there is a wall between the player and the enemy, the enemy will not move.
            local start = point2i(
                self.drawer.position.x / TILE_SIZE,
                self.drawer.position.y / TILE_SIZE
            )
            local goal = point2i(
                player.drawer.position.x / TILE_SIZE,
                player.drawer.position.y / TILE_SIZE
            )

            local min_x = math.min(start.x, goal.x)
            local max_x = math.max(start.x, goal.x)
            local min_y = math.min(start.y, goal.y)
            local max_y = math.max(start.y, goal.y)
            for i = min_x, max_x do
                if self.map:at(i, start.y) < MAP_CHIP_WALKABLE then
                    return
                end
            end
            for i = min_y, max_y do
                if self.map:at(start.x, i) < MAP_CHIP_WALKABLE then
                    return
                end
            end
            self.drawer.rotation = vector3(0, 0,
                math.deg(
                    -math.atan(
                        player.drawer.position.x -
                        self.drawer.position.x,
                        player.drawer.position.y -
                        self.drawer.position.y)))
            if self.bfs:find_path(start, goal) then
                local path = self.bfs:trace()
                path = self.bfs:trace()

                local dir = vector2(
                    path.x * TILE_SIZE - self.drawer.position.x,
                    path.y * TILE_SIZE - self.drawer.position.y)
                if dir.x < -1 then
                    dir.x = -1
                end
                if dir.x > 1 then
                    dir.x = 1
                end
                if dir.y < -1 then
                    dir.y = -1
                end
                if dir.y > 1 then
                    dir.y = 1
                end

                self.drawer.position.x =
                    self.drawer.position.x +
                    dir.x * scene.delta_time() * self.speed
                self.drawer.position.y =
                    self.drawer.position.y +
                    dir.y * scene.delta_time() * self.speed
            else
                self.drawer.position.x =
                    self.drawer.position.x + scene.delta_time() * self.speed *
                    self.get_forward_z(self.drawer).x
                self.drawer.position.y =
                    self.drawer.position.y + scene.delta_time() * self.speed *
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
                    if player.hp <= 0 then
                        player.hp = 0
                    end
                    player:render_text()
                    self.is_collision_first = false
                else
                    self.collision_timer = self.collision_timer + scene.delta_time()
                    if self.collision_timer > self.collision_time then
                        bombed:play()
                        player.hp = player.hp - 10
                        if player.hp <= 0 then
                            player.hp = 0
                        end
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
