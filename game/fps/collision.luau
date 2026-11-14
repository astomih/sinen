local sn = require("@sinen")
local Art = require("./art")

local Collision = {}

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

function Collision.wallDistance(solids: { Art.Box }, origin: sn.Vec3, direction: sn.Vec3, limit: number): number
	for _, box in solids do
		local x0, x1 = slab(origin.x, direction.x, box.lo.x, box.hi.x)
		local y0, y1 = slab(origin.y, direction.y, box.lo.y, box.hi.y)
		local z0, z1 = slab(origin.z, direction.z, box.lo.z, box.hi.z)
		local entry = math.max(x0, y0, z0, 0)
		if entry <= math.min(x1, y1, z1, limit) then
			limit = entry
		end
	end
	return limit
end

function Collision.sphereDistance(origin: sn.Vec3, direction: sn.Vec3, center: sn.Vec3, radius: number): number?
	local offset = center - origin
	local projection = direction:dot(offset)
	local discriminant = projection ^ 2 - offset:dot(offset) + radius ^ 2
	if discriminant < 0 then return nil end
	local root = math.sqrt(discriminant)
	if projection + root < 0 then return nil end
	return math.max(0, projection - root)
end

-- Clip each axis independently so the body slides along obstacles.
function Collision.move(solids: { Art.Box }, feet: sn.Vec3, delta: sn.Vec3, radius: number, height: number): (sn.Vec3, boolean, boolean)
	local x, y, z = feet.x, feet.y, feet.z
	local goalX, goalZ, goalY = x + delta.x, z + delta.z, y + delta.y
	for _, box in solids do
		if y < box.hi.y and y + height > box.lo.y and z + radius > box.lo.z and z - radius < box.hi.z then
			if delta.x > 0 and x <= box.lo.x - radius + 1e-5 then
				goalX = math.min(goalX, box.lo.x - radius)
			elseif delta.x < 0 and x >= box.hi.x + radius - 1e-5 then
				goalX = math.max(goalX, box.hi.x + radius)
			end
		end
	end
	-- Compare the original float position with a small tolerance at contact.
	x = goalX
	for _, box in solids do
		if y < box.hi.y and y + height > box.lo.y and x + radius > box.lo.x and x - radius < box.hi.x then
			if delta.z > 0 and z <= box.lo.z - radius + 1e-5 then
				goalZ = math.min(goalZ, box.lo.z - radius)
			elseif delta.z < 0 and z >= box.hi.z + radius - 1e-5 then
				goalZ = math.max(goalZ, box.hi.z + radius)
			end
		end
	end
	z = goalZ
	for _, box in solids do
		if x + radius > box.lo.x + 1e-5 and x - radius < box.hi.x - 1e-5
			and z + radius > box.lo.z + 1e-5 and z - radius < box.hi.z - 1e-5 then
			if delta.y <= 0 and y >= box.hi.y - 1e-5 then
				goalY = math.max(goalY, box.hi.y)
			elseif delta.y > 0 and y + height <= box.lo.y + 1e-5 then
				goalY = math.min(goalY, box.lo.y - height)
			end
		end
	end
	return sn.Vec3.new(x, goalY, z), delta.y < 0 and goalY > y + delta.y, goalY ~= y + delta.y
end

return Collision
