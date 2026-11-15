local function slab(origin: number, direction: number, lo: number, hi: number): (number, number)
	if math.abs(direction) < 1e-7 then
		if origin < lo or origin > hi then
			return math.huge, -math.huge
		end
		return -math.huge, math.huge
	end
	local a, b = (lo - origin) / direction, (hi - origin) / direction
	return math.min(a, b), math.max(a, b)
end

return slab
