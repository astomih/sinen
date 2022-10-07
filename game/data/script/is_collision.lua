local function is_collision(player, map, map_draw3ds, map_size_x, map_size_y)
    is_collied = false
    player.aabb.max = player.drawer.position:add(
                          player.drawer.scale:mul(player.model.aabb.max))
    player.aabb.min = player.drawer.position:add(
                          player.drawer.scale:mul(player.model.aabb.min))
    for i, v in ipairs(collision_space[math.floor(
                           ((player.drawer.position.y + 1.5)) /
                               (2 * collision_space_division) + 2)][math.floor(
                           ((player.drawer.position.x + 1.5)) /
                               (2 * collision_space_division) + 2)]) do
        if player.aabb:intersects_aabb(v.aabb) then is_collied = true end

    end
    for i, v in ipairs(collision_space[math.floor(
                           ((player.drawer.position.y + 0.5)) /
                               (2 * collision_space_division) + 2)][math.floor(
                           ((player.drawer.position.x + 0.5)) /
                               (2 * collision_space_division) + 2)]) do
        if player.aabb:intersects_aabb(v.aabb) then is_collied = true end

    end
    return is_collied
end
return is_collision

