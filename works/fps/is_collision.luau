local sn = require("@sinen")
local G = require("./global")
local World = require("./world")
local Grid = require("./grid")

local function is_collision(
	position: sn.Vec3,
	aabb: sn.AABB,
	map: Grid.Grid,
	map_draw3ds: { { World.World } },
	map_size_x: number,
	map_size_y: number
)
	local around = sn.Vec2.new(0, 0)
	for i = 1, 9 do
		around.x = math.floor(position.x / G.TILE_SIZE + 0.5) - 1 + i % 3
		around.y = math.floor(position.y / G.TILE_SIZE + 0.5) - 1 + math.floor(i / 3)
		if around.x > 0 and around.y > 0 and around.x <= map_size_x and around.y <= map_size_y then
			if map:at(around.x, around.y) < G.MAP_CHIP_WALKABLE then
				if aabb:intersectsAABB(map_draw3ds[around.y][around.x].aabb) then
					return true
				end
			end
		end
	end
	return false
end

return is_collision
