local function calc_input_vector()
    local input_vector = Vec3(0, 0, 0)
    if keyboard.is_down(keyboard.W) then input_vector.y = input_vector.y + 1.0; end
    if keyboard.is_down(keyboard.S) then input_vector.y = input_vector.y - 1.0; end
    if keyboard.is_down(keyboard.A) then input_vector.x = input_vector.x - 1.0; end
    if keyboard.is_down(keyboard.D) then input_vector.x = input_vector.x + 1.0; end
    return input_vector
end

return calc_input_vector;
