local function calc_input_vector()
    local input_vector = vector3(0, 0, 0)
    if keyboard:is_key_down(keyUP) then input_vector.y = input_vector.y + 1.0; end
    if keyboard:is_key_down(keyDOWN) then input_vector.y = input_vector.y - 1.0; end
    if input_vector.y ~= 0 then return input_vector end
    if keyboard:is_key_down(keyLEFT) then input_vector.x = input_vector.x - 1.0; end
    if keyboard:is_key_down(keyRIGHT) then input_vector.x = input_vector.x + 1.0; end
    return input_vector
end

return calc_input_vector;
