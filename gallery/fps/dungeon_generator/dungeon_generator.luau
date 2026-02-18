--------------------------------------------------------------------
-- dungeon_generator.lua
--------------------------------------------------------------------
local Room = require("./room")
local Corridor = require("./corridor")

local function centerOf(room)
	local p = room:get_position()
	local s = room:get_size()
	return Vec2.new(p.x + math.floor(s.x / 2), p.y + math.floor(s.y / 2))
end

---@param grid   Grid
---@param floor  integer
---@param wall   integer
---@param player integer
---@param key    integer
---@param stair  integer
local function dungeonGenerator(grid, floor, wall, player, key, stair)
	if grid:width() < 3 or grid:height() < 3 then
		return
	end

	local recreate = false
	local grid_size = Vec2.new(grid:width(), grid:height())

	local min_rooms = 5
	local max_rooms = 14
	local room_count = math.random(min_rooms, max_rooms)

	local min_size = Vec2.new(5, 5)
	local sz = 5 + max_rooms - room_count
	local max_size = Vec2.new(sz, sz)

	------------------------------------------------------------
	-- Room generate
	------------------------------------------------------------
	local rooms = {}
	for i = 1, room_count do
		rooms[i] = Room:new(grid_size, min_size, max_size)
	end

	-- init
	for y = 1, grid:height() do
		for x = 1, grid:width() do
			grid:set(x, y, wall)
		end
	end

	-- fill floor
	for _, r in ipairs(rooms) do
		r:fill(grid, floor)
	end

	-- sort x for corridor
	table.sort(rooms, function(a, b)
		return a:get_position().x < b:get_position().x
	end)

	------------------------------------------------------------
	-- Key / Stairs / Player placing
	------------------------------------------------------------
	local stairs_pos = centerOf(rooms[#rooms])
	local key_pos = centerOf(rooms[math.floor(#rooms / 2) + 1])
	local player_pos = centerOf(rooms[1])

	grid:set(stairs_pos.x, stairs_pos.y, stair)
	grid:set(key_pos.x, key_pos.y, key)
	grid:set(player_pos.x, player_pos.y, player)

	------------------------------------------------------------
	-- corridor
	------------------------------------------------------------
	local corridor = Corridor:new()
	corridor:connect(grid, rooms, floor)

	------------------------------------------------------------
	-- Check
	------------------------------------------------------------
	local function reachable(a, b)
		local bfs = BFSGrid.new(grid)
		return bfs:findPath(a, b)
	end

	if
		not (reachable(player_pos, key_pos) and reachable(key_pos, stairs_pos) and reachable(player_pos, stairs_pos))
	then
		recreate = true
	end

	if recreate then
		dungeonGenerator(grid, floor, wall, player, key, stair)
	end
end

return dungeonGenerator
