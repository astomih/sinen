local function is_collision(player, map, map_draw3ds, map_size_x, map_size_y)
    player.aabb.max = player.drawer.position:add(
        player.drawer.scale:mul(player.model.aabb.max))
    player.aabb.min = player.drawer.position:add(
        player.drawer.scale:mul(player.model.aabb.min))
    for i = 1, 9 do
        local around = point2i(0, 0)
        around.x = math.floor(player.drawer.position.x / 2 + 0.5) - 1 + i % 3
        around.y = math.floor(player.drawer.position.y / 2 + 0.5) - 1 + math.floor(i / 3)
        if around.x > 0 and around.y > 0 and around.x <= map_size_x and around.y <= map_size_y then
            local ab = aabb()
            ab.max = map_draw3ds[around.y][around.x].aabb.max
            ab.min = map_draw3ds[around.y][around.x].aabb.min
            if collision.aabb_aabb(player.aabb, ab) then return true end
        end

    end
    return false
end

return is_collision
