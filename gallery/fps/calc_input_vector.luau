local function calc_input_vector(): Vec3
	local input_vector = Vec3.new(0, 0, 0)
	if Keyboard.isDown(Keyboard.W) then
		input_vector.y = input_vector.y + 1.0
	end
	if Keyboard.isDown(Keyboard.S) then
		input_vector.y = input_vector.y - 1.0
	end
	if Keyboard.isDown(Keyboard.A) then
		input_vector.x = input_vector.x - 1.0
	end
	if Keyboard.isDown(Keyboard.D) then
		input_vector.x = input_vector.x + 1.0
	end
	return input_vector
end

return calc_input_vector
