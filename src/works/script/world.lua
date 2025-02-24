local function world()
    local object = {
        position = Vector3(0, 0, 0),
        rotation = Vector3(0, 0, 0),
        scale = Vector2(1, 1, 1),
        aabb = AABB()
    }
    return object
end

return world
