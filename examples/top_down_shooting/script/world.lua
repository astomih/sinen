local function world()
    local object = {
        position = Vec3(0, 0, 0),
        rotation = Vec3(0, 0, 0),
        scale = Vec2(1, 1, 1),
        aabb = AABB()
    }
    return object
end

return world
