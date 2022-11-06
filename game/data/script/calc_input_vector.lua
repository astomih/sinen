local function calc_input_vector()
    local input_vector = vector3(0, 0, 0)
    if keyboard:is_key_down(keyW) then input_vector.y = input_vector.y + 1.0; end
    if keyboard:is_key_down(keyS) then input_vector.y = input_vector.y - 1.0; end
    if keyboard:is_key_down(keyA) then input_vector.x = input_vector.x - 1.0; end
    if keyboard:is_key_down(keyD) then input_vector.x = input_vector.x + 1.0; end
    return input_vector
end

return calc_input_vector;
