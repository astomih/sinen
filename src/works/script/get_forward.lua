-- Get the objects forward vector
local function get_forward(drawer)
    return vector3(-math.sin(math.rad(drawer.rotation.z)),
        math.cos(math.rad(-drawer.rotation.z)), 1)
end

return get_forward
