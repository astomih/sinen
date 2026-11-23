local sn = require("@sinen")

local function slab(origin: number, direction: number, lo: number, hi: number): sn.Vec2
	if math.abs(direction) < 1e-7 then
		if origin < lo or origin > hi then
			return sn.Vec2.new(math.huge, -math.huge)
		end
		return sn.Vec2.new(-math.huge, math.huge)
	end
	local a, b = (lo - origin) / direction, (hi - origin) / direction
	return sn.Vec2.new(math.min(a, b), math.max(a, b))
end

return slab
