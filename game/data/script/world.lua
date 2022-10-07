local function world()
    local object = {
        position = vector3(0, 0, 0),
        rotation = vector3(0, 0, 0),
        scale = vector3(1, 1, 1),
        aabb = {}
    }
    return object
end

return world
